// quant_feed: read-only IBKR market data client. Connects to Gateway on localhost,
// subscribes the universe, optionally records ticks, and reports permissions.
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <thread>

#include "quant/ib_socket.hpp"
#include "quant/ib_wire.hpp"

using namespace quant::ib;
namespace ch = std::chrono;

namespace {

volatile std::sig_atomic_t g_stop = 0;
void on_signal(int) { g_stop = 1; }

struct Quote {
    double bid = 0, ask = 0, last = 0, close = 0;
    long ticks = 0;
    int data_type = 0, error = 0;
    std::string error_text;
};

const char* type_name(int t) {
    switch (t) { case 1: return "realtime"; case 2: return "frozen"; case 3: return "delayed"; case 4: return "delayed-frozen"; default: return "none"; }
}

std::string account_kind(const std::string& accounts) {
    std::string first = accounts.substr(0, accounts.find(','));
    return first.rfind("D", 0) == 0 ? "paper" : "live";
}

int run(int port, int mode, int seconds, const std::string& record, const std::string& universe) {
    std::vector<std::string> symbols;
    { std::ifstream f(universe); if (!f) throw std::runtime_error("cannot open " + universe); std::string s; while (f >> s) symbols.push_back(s); }
    symbols.push_back("SPY");

    int fd = connect_local(port);
    Framer framer;
    char buf[65536];
    std::string body;
    const int server = open_session(fd, framer, 17);

    std::vector<Quote> quotes(symbols.size());
    std::map<int, std::pair<long, std::string>> global_errors;
    std::ofstream out;
    if (record != "none") out.open(record, std::ios::app);
    std::string account;
    bool ready = false, subscribed = false;
    size_t next_sub = 0;
    long messages = 0;
    std::vector<int64_t> latency;
    latency.reserve(1 << 20);
    const auto start = ch::steady_clock::now();
    auto last_sub = start;

    auto handle = [&](const Event& e) {
        const bool valid = e.id >= 0 && static_cast<size_t>(e.id) < quotes.size();
        switch (e.kind) {
            case Kind::NextValidId: ready = true; break;
            case Kind::ManagedAccounts: account = e.text; break;
            case Kind::MarketDataType: if (valid) quotes[static_cast<size_t>(e.id)].data_type = e.data_type; break;
            case Kind::Error:
                if (valid) { quotes[static_cast<size_t>(e.id)].error = e.code; quotes[static_cast<size_t>(e.id)].error_text = e.text; }
                { auto& g = global_errors[e.code]; g.first += 1; g.second = e.text; }
                break;
            case Kind::Price:
                if (valid) {
                    Quote& q = quotes[static_cast<size_t>(e.id)];
                    ++q.ticks;
                    if (e.tick == 1 || e.tick == 66) q.bid = e.value;
                    else if (e.tick == 2 || e.tick == 67) q.ask = e.value;
                    else if (e.tick == 4 || e.tick == 68) q.last = e.value;
                    else if (e.tick == 9 || e.tick == 75) q.close = e.value;
                }
                break;
            case Kind::Size: if (valid) ++quotes[static_cast<size_t>(e.id)].ticks; break;
            case Kind::Other: case Kind::String: case Kind::Generic: break;
        }
    };

    while (!g_stop && ch::steady_clock::now() - start < ch::seconds(seconds)) {
        const auto now = ch::steady_clock::now();
        if (ready && !subscribed && now - last_sub >= ch::milliseconds(25)) {
            if (next_sub == 0) send_all(fd, req_market_data_type(mode));
            send_all(fd, req_stock_quotes(static_cast<int>(next_sub), symbols[next_sub]));
            last_sub = now;
            if (++next_sub == symbols.size()) subscribed = true;
        }
        pollfd p{fd, POLLIN, 0};
        int r = ::poll(&p, 1, ready && !subscribed ? 5 : 100);
        if (r < 0) break;
        if (r == 0) continue;
        ssize_t n = ::recv(fd, buf, sizeof buf, 0);
        if (n <= 0) { std::cerr << "connection lost\n"; break; }
        framer.feed(buf, static_cast<size_t>(n));
        while (true) {
            const auto t0 = ch::steady_clock::now();
            if (!framer.next(body)) break;
            Event e = decode(split_fields(body));
            handle(e);
            ++messages;
            if (latency.size() < latency.capacity()) latency.push_back(ch::duration_cast<ch::nanoseconds>(ch::steady_clock::now() - t0).count());
            if (out.is_open() && (e.kind == Kind::Price || e.kind == Kind::Size) && e.id >= 0 && static_cast<size_t>(e.id) < symbols.size()) {
                out << ch::duration_cast<ch::milliseconds>(ch::system_clock::now().time_since_epoch()).count() << ',' << symbols[static_cast<size_t>(e.id)] << ','
                    << (e.kind == Kind::Price ? 'p' : 's') << ',' << e.tick << ',' << e.value << '\n';
            }
        }
    }
    for (size_t i = 0; i < next_sub; ++i) { send_all(fd, cancel_quotes(static_cast<int>(i))); std::this_thread::sleep_for(ch::milliseconds(5)); }
    ::close(fd);

    std::map<std::string, int> by_type;
    int with_price = 0;
    std::vector<std::string> problems;
    for (size_t i = 0; i < symbols.size(); ++i) {
        const Quote& q = quotes[i];
        by_type[type_name(q.data_type)] += 1;
        const bool priced = q.bid > 0 || q.last > 0 || q.close > 0;
        with_price += priced;
        if (!priced) problems.push_back(symbols[i] + (q.error ? "(" + std::to_string(q.error) + ")" : "(no data)"));
    }
    std::printf("account_type=%s server_version=%d requested=%s symbols=%zu with_price=%d messages=%ld\n", account_kind(account).c_str(), server, type_name(mode), symbols.size(), with_price, messages);
    for (auto& [t, c] : by_type) std::printf("  data_type %-14s %d\n", t.c_str(), c);
    if (!problems.empty()) { std::printf("  unpriced:"); for (auto& s : problems) std::printf(" %s", s.c_str()); std::printf("\n"); }
    for (auto& [code, g] : global_errors) std::printf("  code %-6d x%-4ld %.110s\n", code, g.first, g.second.c_str());
    const Quote& spy = quotes.back();
    std::printf("  SPY bid=%.2f ask=%.2f last=%.2f prior_close=%.2f type=%s\n", spy.bid, spy.ask, spy.last, spy.close, type_name(spy.data_type));
    if (!latency.empty()) {
        std::sort(latency.begin(), latency.end());
        auto q = [&](double f) { return latency[static_cast<size_t>(f * static_cast<double>(latency.size() - 1))]; };
        std::printf("  decode_to_book_ns p50=%lld p99=%lld max=%lld (in-process only, n=%zu)\n", (long long)q(.5), (long long)q(.99), (long long)latency.back(), latency.size());
    }
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    int port = 4001, mode = 1, seconds = 20;
    std::string record = "none", universe = "config/universe.txt";
    for (int i = 1; i + 1 < argc; i += 2) {
        std::string k = argv[i], v = argv[i + 1];
        if (k == "--port") port = std::stoi(v);
        else if (k == "--seconds") seconds = std::stoi(v);
        else if (k == "--record") record = v;
        else if (k == "--universe") universe = v;
        else if (k == "--mode") mode = v == "realtime" ? 1 : v == "frozen" ? 2 : v == "delayed" ? 3 : v == "delayed-frozen" ? 4 : 0;
        else { std::cerr << "unknown option " << k << "\n"; return 2; }
    }
    if (mode == 0) { std::cerr << "bad --mode\n"; return 2; }
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);
    std::signal(SIGPIPE, SIG_IGN);
    try { return run(port, mode, seconds, record, universe); }
    catch (const std::exception& e) { std::cerr << "feed error: " << e.what() << "\n"; return 1; }
}
