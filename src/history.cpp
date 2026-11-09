// quant_history: downloads IBKR split-adjusted daily TRADES bars, one symbol at a
// time. Read-only: it can request history and nothing else.
#include <poll.h>

#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <thread>

#include "quant/ib_socket.hpp"

using namespace quant::ib;
namespace ch = std::chrono;

int main(int argc, char** argv) {
    int port = 4001;
    std::string duration = "8 Y", output = "data/ibkr_daily.csv", universe = "config/universe.txt";
    for (int i = 1; i + 1 < argc; i += 2) {
        std::string k = argv[i], v = argv[i + 1];
        if (k == "--port") port = std::stoi(v);
        else if (k == "--duration") duration = v;
        else if (k == "--output") output = v;
        else if (k == "--universe") universe = v;
        else { std::cerr << "unknown option " << k << "\n"; return 2; }
    }
    try {
        std::vector<std::string> symbols;
        { std::ifstream f(universe); if (!f) throw std::runtime_error("cannot open " + universe); std::string s; while (f >> s) symbols.push_back(s); }
        symbols.push_back("SPY");
        int fd = connect_local(port);
        Framer framer;
        open_session(fd, framer, 18);
        std::ofstream out(output, std::ios::trunc);
        out << "date,symbol,open,high,low,close,volume\n";
        char buf[1 << 16];
        std::string body;
        size_t done = 0;
        long bars = 0;
        std::map<int, int> errors;
        std::vector<std::string> failed;
        bool ready = false;
        for (size_t i = 0; i < symbols.size(); ++i) {
            bool sent = false, finished = false;
            auto started = ch::steady_clock::now();
            while (!finished) {
                if (ready && !sent) { send_all(fd, req_daily_history(static_cast<int>(i), symbols[i], duration)); sent = true; started = ch::steady_clock::now(); }
                if (sent && ch::steady_clock::now() - started > ch::seconds(60)) { failed.push_back(symbols[i] + "(timeout)"); break; }
                pollfd p{fd, POLLIN, 0};
                if (::poll(&p, 1, 200) <= 0) continue;
                ssize_t n = ::recv(fd, buf, sizeof buf, 0);
                if (n <= 0) throw std::runtime_error("connection lost");
                framer.feed(buf, static_cast<size_t>(n));
                while (framer.next(body)) {
                    auto f = split_fields(body);
                    if (f.empty()) continue;
                    const int msg = to_int(f[0]);
                    if (msg == 9) ready = true;
                    else if (msg == 4 && f.size() >= 5) {
                        const int id = to_int(f[2]), code = to_int(f[3]);
                        errors[code] += 1;
                        if (id == static_cast<int>(i) && id >= 0) { failed.push_back(symbols[i] + "(" + std::to_string(code) + ")"); finished = true; }
                    } else if (msg == 17 && f.size() >= 5 && to_int(f[1]) == static_cast<int>(i)) {
                        const int count = to_int(f[4]);
                        size_t k = 5;
                        for (int b = 0; b < count && k + 8 <= f.size(); ++b, k += 8) {
                            out << std::string(f[k]) << ',' << symbols[i] << ',' << std::string(f[k+1]) << ',' << std::string(f[k+2]) << ',' << std::string(f[k+3]) << ',' << std::string(f[k+4]) << ',' << std::string(f[k+5]) << '\n';
                            ++bars;
                        }
                        ++done;
                        finished = true;
                    }
                }
            }
            std::this_thread::sleep_for(ch::milliseconds(250));
        }
        ::close(fd);
        std::printf("symbols_ok=%zu/%zu bars=%ld\n", done, symbols.size(), bars);
        if (!failed.empty()) { std::printf("failed:"); for (auto& s : failed) std::printf(" %s", s.c_str()); std::printf("\n"); }
        for (auto& [c, n] : errors) std::printf("  code %d x%d\n", c, n);
        return 0;
    } catch (const std::exception& e) { std::cerr << "history error: " << e.what() << "\n"; return 1; }
}
