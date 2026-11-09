// quant_dash: read-only live OTC dashboard. One thread and one poll loop: Gateway frames
// in, per-symbol analytics, server-sent events out to a browser on this machine only.
// It speaks ib_wire.hpp, which has no way to express an order.
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>

#include "quant/ib_socket.hpp"
#include "quant/ib_wire.hpp"
#include "quant/otc_book.hpp"

using namespace quant::ib;
using namespace quant::otc;
namespace ch = std::chrono;

namespace {

volatile std::sig_atomic_t g_stop = 0;
void on_signal(int) { g_stop = 1; }

constexpr int kHistBase = 1000;
constexpr size_t kMaxRequest = 8192, kMaxClientBuffer = 8u << 20;

int64_t ms_now() { return ch::duration_cast<ch::milliseconds>(ch::system_clock::now().time_since_epoch()).count(); }
int64_t ns_now() { return ch::duration_cast<ch::nanoseconds>(ch::steady_clock::now().time_since_epoch()).count(); }

struct Session { const char* name; double fraction; };

// New York time, set at startup. Exchange holidays are not known here.
Session session_now() {
    std::time_t t = std::time(nullptr);
    std::tm lt{};
    localtime_r(&t, &lt);
    if (lt.tm_wday == 0 || lt.tm_wday == 6) return {"CLOSED", 0};
    const double m = lt.tm_hour * 60 + lt.tm_min + lt.tm_sec / 60.0;
    if (m < 570) return {"PRE", 0};
    if (m < 960) return {"OPEN", (m - 570) / 390};
    return {"POST", 1};
}

void put(std::string& o, const char* k, double v) {
    o += '"'; o += k; o += "\":";
    if (!std::isfinite(v)) { o += "null,"; return; }
    char b[40];
    int n = std::snprintf(b, sizeof b, "%.10g,", v);
    o.append(b, static_cast<size_t>(n));
}
void put_int(std::string& o, const char* k, int64_t v) { o += '"'; o += k; o += "\":"; o += std::to_string(v); o += ','; }
void escape(std::string& o, std::string_view s) {
    o += '"';
    for (char c : s) {
        if (c == '"' || c == '\\') { o += '\\'; o += c; }
        else if (static_cast<unsigned char>(c) < 0x20) o += ' ';
        else o += c;
    }
    o += '"';
}
void put_str(std::string& o, const char* k, std::string_view v) { o += '"'; o += k; o += "\":"; escape(o, v); o += ','; }
void close_obj(std::string& o, char c) { if (!o.empty() && o.back() == ',') o.pop_back(); o += c; }
void num(std::string& o, double v) { if (!std::isfinite(v)) { o += "null,"; return; } char b[40]; o.append(b, static_cast<size_t>(std::snprintf(b, sizeof b, "%.10g,", v))); }

struct Slot {
    std::string symbol;
    bool active = true, dirty = false, hist_done = false;
    int data_type = 0, error = 0;
    std::string error_text, hist_status = "PENDING";
    int64_t dirty_ns = 0;
    Book book;
    std::vector<Daily> daily;
    std::vector<std::string> dates;
};

struct Client {
    int fd = -1;
    std::string in, out;
    size_t sent = 0;
    bool sse = false, closing = false, dead = false;
};

struct Options {
    int gw_port = 4001, http_port = 8877, client_id = 19, mode = 1, flush_ms = 25, max_lines = 80;
    double ticket = 500;
    std::string watch = "config/otc_watchlist.txt", web = "web/otc.html";
};

bool valid_symbol(const std::string& s) {
    if (s.empty() || s.size() > 12) return false;
    return std::all_of(s.begin(), s.end(), [](char c) { return (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.'; });
}

std::string header(const std::string& req, const char* name) {
    std::string lower = req;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    const std::string key = std::string("\r\n") + name + ":";
    size_t p = lower.find(key);
    if (p == std::string::npos) return "";
    p += key.size();
    size_t e = req.find("\r\n", p);
    while (p < e && req[p] == ' ') ++p;
    return req.substr(p, e - p);
}

class Dash {
public:
    explicit Dash(Options o) : opt_(std::move(o)) {}

    int run() {
        load_watchlist();
        listen_http();
        std::printf("quant_dash: http://127.0.0.1:%d  (gateway port %d, client id %d, read-only)\n", opt_.http_port, opt_.gw_port, opt_.client_id);
        std::fflush(stdout);
        int64_t next_second = 0;
        while (!g_stop) {
            std::vector<pollfd> p;
            p.push_back({listen_fd_, POLLIN, 0});
            const bool gw = gw_fd_ >= 0;
            if (gw) p.push_back({gw_fd_, POLLIN, 0});
            const size_t first_client = p.size();
            for (auto& c : clients_) p.push_back({c->fd, static_cast<short>(POLLIN | (pending(*c) ? POLLOUT : 0)), 0});

            const int timeout = any_dirty_ || !sub_queue_.empty() || !hist_queue_.empty() ? std::max(1, opt_.flush_ms / 2) : 100;
            if (::poll(p.data(), static_cast<nfds_t>(p.size()), timeout) < 0 && errno != EINTR) break;

            if (gw && (p[1].revents & (POLLIN | POLLHUP | POLLERR))) read_gateway();
            if (p[0].revents & POLLIN) accept_clients();
            for (size_t i = 0; i < clients_.size() && first_client + i < p.size(); ++i) {
                if (p[first_client + i].revents & (POLLIN | POLLHUP)) read_client(*clients_[i]);
                if (p[first_client + i].revents & POLLOUT) write_client(*clients_[i]);
            }
            clients_.erase(std::remove_if(clients_.begin(), clients_.end(), [](const std::unique_ptr<Client>& c) {
                if (c->dead) ::close(c->fd);
                return c->dead;
            }), clients_.end());

            pace_requests();
            const int64_t now = ms_now();
            if (any_dirty_ && now - last_flush_ >= opt_.flush_ms) flush(now);
            if (now >= next_second) { next_second = now + 1000; each_second(now); }
        }
        if (gw_fd_ >= 0) ::close(gw_fd_);
        for (auto& c : clients_) ::close(c->fd);
        ::close(listen_fd_);
        return 0;
    }

private:
    bool pending(const Client& c) const { return c.sent < c.out.size(); }

    void load_watchlist() {
        std::ifstream f(opt_.watch);
        std::string s;
        const int64_t now = ms_now();
        while (f >> s) {
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
            if (valid_symbol(s) && !index_.count(s) && static_cast<int>(slots_.size()) < opt_.max_lines) add_slot(s, now);
        }
    }

    void save_watchlist() {
        std::ofstream f(opt_.watch, std::ios::trunc);
        for (auto& s : slots_) if (s->active) f << s->symbol << '\n';
    }

    int add_slot(const std::string& symbol, int64_t now) {
        auto s = std::make_unique<Slot>();
        s->symbol = symbol;
        s->book.start(now);
        slots_.push_back(std::move(s));
        const int id = static_cast<int>(slots_.size()) - 1;
        index_[symbol] = id;
        return id;
    }

    int active_lines() const { int n = 0; for (auto& s : slots_) n += s->active; return n; }

    // --- Gateway ---------------------------------------------------------------
    void connect_gateway(int64_t now) {
        last_connect_ = now;
        int fd = -1;
        try {
            fd = connect_local(opt_.gw_port);
            timeval tv{5, 0};
            ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
            framer_ = Framer();
            open_session(fd, framer_, opt_.client_id);
            gw_fd_ = fd;
            ready_ = false;
            gw_error_.clear();
        } catch (const std::exception& e) {
            if (fd >= 0) ::close(fd);
            gw_error_ = e.what();
        }
    }

    void drop_gateway(const std::string& why) {
        if (gw_fd_ >= 0) ::close(gw_fd_);
        gw_fd_ = -1;
        ready_ = false;
        sub_queue_.clear();
        hist_queue_.clear();
        gw_error_ = why;
        for (auto& s : slots_) s->data_type = 0;
    }

    void read_gateway() {
        char buf[1 << 16];
        ssize_t n = ::recv(gw_fd_, buf, sizeof buf, MSG_DONTWAIT);
        if (n == 0 || (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) { drop_gateway(last_error_text_.empty() ? "gateway closed the connection" : last_error_text_); return; }
        if (n < 0) return;
        try {
            framer_.feed(buf, static_cast<size_t>(n));
            std::string body;
            while (true) {
                const int64_t t0 = ns_now();
                if (!framer_.next(body)) break;
                handle(split_fields(body), t0);
                if (engine_ns_.size() < 4096) engine_ns_.push_back(ns_now() - t0);
                ++messages_;
            }
        } catch (const std::exception& e) {
            drop_gateway(std::string("protocol error: ") + e.what());
        }
    }

    void mark(Slot& s, int64_t t0) {
        if (!s.dirty) { s.dirty = true; s.dirty_ns = t0; }
        any_dirty_ = true;
    }

    Slot* live_slot(int id) {
        if (id < 0 || static_cast<size_t>(id) >= slots_.size() || !slots_[static_cast<size_t>(id)]->active) return nullptr;
        return slots_[static_cast<size_t>(id)].get();
    }

    void handle(const std::vector<std::string_view>& f, int64_t t0) {
        if (f.empty()) return;
        if (to_int(f[0]) == 17) { handle_history(f); return; }
        const Event e = decode(f);
        const int64_t now = ms_now();
        switch (e.kind) {
            case Kind::NextValidId:
                ready_ = true;
                send_all(gw_fd_, req_market_data_type(opt_.mode));
                for (size_t i = 0; i < slots_.size(); ++i) if (slots_[i]->active) enqueue(static_cast<int>(i));
                break;
            case Kind::ManagedAccounts:
                account_kind_ = e.text.rfind("D", 0) == 0 ? "PAPER" : "LIVE";  // The account number itself never leaves this function.
                break;
            case Kind::MarketDataType:
                if (Slot* s = live_slot(e.id)) { s->data_type = e.data_type; mark(*s, t0); }
                break;
            case Kind::Price:
                if (Slot* s = live_slot(e.id)) { s->book.on_price(e.tick, e.value, now); clear_error(*s); mark(*s, t0); }
                break;
            case Kind::Size:
                if (Slot* s = live_slot(e.id)) { s->book.on_size(e.tick, e.value, now); clear_error(*s); mark(*s, t0); }
                break;
            case Kind::String:
                if (e.tick == 48) if (Slot* s = live_slot(e.id)) { s->book.on_rt_volume(e.text, now); mark(*s, t0); }
                break;
            case Kind::Generic:
                if (e.tick == 49) if (Slot* s = live_slot(e.id)) { s->book.on_halt(e.value); mark(*s, t0); }
                break;
            case Kind::Error: on_error(e, t0); break;
            case Kind::Other: break;
        }
    }

    void clear_error(Slot& s) { if (s.error && s.error != 10167) { s.error = 0; s.error_text.clear(); } }

    void on_error(const Event& e, int64_t t0) {
        if (e.id >= kHistBase) {
            if (static_cast<size_t>(e.id - kHistBase) < slots_.size()) { auto& s = *slots_[static_cast<size_t>(e.id - kHistBase)]; s.hist_status = "ERR " + std::to_string(e.code); s.hist_done = true; }
            return;
        }
        if (Slot* s = live_slot(e.id)) { s->error = e.code; s->error_text = e.text; mark(*s, t0); }
        else if (e.id < 0 && e.code != 2104 && e.code != 2106 && e.code != 2158 && e.code != 2107 && e.code != 2108) last_error_text_ = std::to_string(e.code) + ": " + e.text;
    }

    void handle_history(const std::vector<std::string_view>& f) {
        if (f.size() < 5) return;
        const int id = to_int(f[1]);
        if (id < kHistBase || static_cast<size_t>(id - kHistBase) >= slots_.size()) return;
        Slot& s = *slots_[static_cast<size_t>(id - kHistBase)];
        const int count = to_int(f[4]);
        s.daily.clear(); s.dates.clear();
        size_t k = 5;
        for (int b = 0; b < count && k + 8 <= f.size(); ++b, k += 8) {
            s.dates.emplace_back(f[k]);
            s.daily.push_back({to_double(f[k + 1]), to_double(f[k + 2]), to_double(f[k + 3]), to_double(f[k + 4]), to_double(f[k + 5])});
        }
        s.book.set_daily(daily_stats(s.daily));
        s.hist_status = s.book.daily().ok ? "OK" : "SHORT";
        s.hist_done = true;
        mark(s, ns_now());
    }

    void enqueue(int id) {
        slots_[static_cast<size_t>(id)]->book.start(ms_now());
        sub_queue_.push_back(id);
        if (!slots_[static_cast<size_t>(id)]->hist_done) hist_queue_.push_back(id);
    }

    void pace_requests() {
        if (gw_fd_ < 0 || !ready_) return;
        const int64_t now = ms_now();
        try {
            if (!sub_queue_.empty() && now - last_sub_ >= 25) {
                const int id = sub_queue_.front(); sub_queue_.pop_front();
                if (Slot* s = live_slot(id)) send_all(gw_fd_, req_quotes_with_trades(id, s->symbol));
                last_sub_ = now;
            }
            if (!hist_queue_.empty() && sub_queue_.empty() && now - last_hist_ >= 500) {
                const int id = hist_queue_.front(); hist_queue_.pop_front();
                if (Slot* s = live_slot(id)) send_all(gw_fd_, req_daily_history(kHistBase + id, s->symbol, "1 Y"));
                last_hist_ = now;
            }
        } catch (const std::exception& e) { drop_gateway(e.what()); }
    }

    // --- Output ----------------------------------------------------------------
    void row(std::string& o, Slot& s, int64_t now) {
        const Metrics m = s.book.metrics(now, opt_.ticket, session_.fraction);
        o += '{';
        put_str(o, "s", s.symbol); put_int(o, "dt", s.data_type); put_int(o, "err", s.error); put_str(o, "errt", s.error_text); put_str(o, "hist", s.hist_status);
        put(o, "bid", m.bid); put(o, "ask", m.ask); put(o, "bsz", m.bid_sz); put(o, "asz", m.ask_sz); put(o, "last", m.last); put(o, "lsz", m.last_sz);
        put(o, "open", m.open); put(o, "high", m.high); put(o, "low", m.low); put(o, "close", m.close); put(o, "vol", m.volume); put(o, "vwap", m.vwap);
        put(o, "mid", m.mid); put(o, "spr", m.spread); put(o, "sprp", m.spread_pct); put(o, "micro", m.micro); put(o, "imb", m.imbalance); put(o, "chg", m.change_pct); put(o, "vwd", m.vwap_dev_pct); put(o, "dvol", m.dollar_vol);
        put(o, "bn", m.bid_notional); put(o, "an", m.ask_notional); put(o, "tws", m.tw_spread_pct); put(o, "two", m.two_sided_pct); put(o, "eff", m.eff_spread_pct); put(o, "rv", m.rv_1m_bps); put(o, "m5", m.mid_5m_pct);
        put(o, "cost", m.cost_pct); put(o, "fill", m.fill_pct); put(o, "pace", m.pace); put(o, "rng", m.range_pos);
        auto flow = [&](const char* p, const Flow& f) { std::string k(p); put(o, (k + "b").c_str(), f.buy); put(o, (k + "s").c_str(), f.sell); put(o, (k + "t").c_str(), f.total); put_int(o, (k + "n").c_str(), f.trades); };
        flow("f1", m.flow_1m); flow("f5", m.flow_5m); flow("f15", m.flow_15m);
        put_int(o, "qpm", m.quotes_per_min); put_int(o, "halt", m.halted); put_int(o, "tq", m.last_quote_ms); put_int(o, "tt", m.last_trade_ms);
        put_str(o, "g", std::string_view(&m.grade, 1));
        const auto& d = s.book.daily();
        put(o, "adv", d.adv20); put(o, "dadv", d.dollar_adv20); put(o, "atr", d.atr14_pct); put(o, "rv20", d.rv20_ann_pct); put(o, "hi", d.high_1y); put(o, "lo", d.low_1y); put_int(o, "zd", d.zero_days60);
        o += "\"fl\":[";
        for (auto& x : m.flags) { escape(o, x); o += ','; }
        close_obj(o, ']');
        o += '}';
    }

    void broadcast(const std::string& json) {
        for (auto& c : clients_) {
            if (!c->sse || c->dead) continue;
            c->out += "data: "; c->out += json; c->out += "\n\n";
            if (c->out.size() - c->sent > kMaxClientBuffer) { c->dead = true; continue; }
            write_client(*c);
        }
    }

    void flush(int64_t now) {
        std::string o = "{\"k\":\"rows\",\"rows\":[";
        std::vector<int64_t> started;
        for (auto& s : slots_) {
            if (!s->dirty || !s->active) continue;
            row(o, *s, now);
            o += ',';
            started.push_back(s->dirty_ns);
            s->dirty = false;
        }
        close_obj(o, ']');
        o += '}';
        any_dirty_ = false;
        last_flush_ = now;
        if (started.empty()) return;
        broadcast(o);
        const int64_t t = ns_now();
        for (int64_t s : started) if (wire_ns_.size() < 4096) wire_ns_.push_back(t - s);
    }

    std::string status(int64_t now) {
        auto pct = [](std::vector<int64_t> v, double q) { if (v.empty()) return kNaN; std::sort(v.begin(), v.end()); return static_cast<double>(v[static_cast<size_t>(q * static_cast<double>(v.size() - 1))]); };
        std::string o = "{\"k\":\"status\",";
        put_int(o, "t", now);
        put_int(o, "gw", gw_fd_ >= 0 && ready_);
        put_str(o, "acct", account_kind_);
        put_str(o, "gwerr", gw_error_);
        put_str(o, "msg", last_error_text_);
        put_str(o, "sess", session_.name);
        put_int(o, "lines", active_lines());
        put_int(o, "maxl", opt_.max_lines);
        put_int(o, "rate", messages_ - last_messages_);
        put_int(o, "clients", std::count_if(clients_.begin(), clients_.end(), [](const auto& c) { return c->sse; }));
        put(o, "ticket", opt_.ticket);
        put(o, "e50", pct(engine_ns_, .5)); put(o, "e99", pct(engine_ns_, .99)); put(o, "w50", pct(wire_ns_, .5)); put(o, "w99", pct(wire_ns_, .99));
        close_obj(o, '}');
        return o;
    }

    void each_second(int64_t now) {
        session_ = session_now();
        for (auto& s : slots_) if (s->active) s->book.clock(now);
        if (gw_fd_ < 0 && now - last_connect_ >= 5000) connect_gateway(now);
        broadcast(status(now));
        last_messages_ = messages_;
        if (engine_ns_.size() >= 4096) engine_ns_.erase(engine_ns_.begin(), engine_ns_.begin() + 2048);
        if (wire_ns_.size() >= 4096) wire_ns_.erase(wire_ns_.begin(), wire_ns_.begin() + 2048);
    }

    // --- HTTP (loopback only) ---------------------------------------------------
    void listen_http() {
        listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd_ < 0) throw std::runtime_error("socket failed");
        int one = 1;
        ::setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
        sockaddr_in a{};
        a.sin_family = AF_INET;
        a.sin_port = htons(static_cast<uint16_t>(opt_.http_port));
        a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (::bind(listen_fd_, reinterpret_cast<sockaddr*>(&a), sizeof a) != 0) throw std::runtime_error("cannot bind 127.0.0.1:" + std::to_string(opt_.http_port));
        if (::listen(listen_fd_, 16) != 0) throw std::runtime_error("listen failed");
        ::fcntl(listen_fd_, F_SETFL, O_NONBLOCK);
    }

    void accept_clients() {
        while (true) {
            int fd = ::accept(listen_fd_, nullptr, nullptr);
            if (fd < 0) return;
            ::fcntl(fd, F_SETFL, O_NONBLOCK);
            int one = 1;
            ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
            auto c = std::make_unique<Client>();
            c->fd = fd;
            clients_.push_back(std::move(c));
        }
    }

    void read_client(Client& c) {
        char buf[8192];
        ssize_t n = ::recv(c.fd, buf, sizeof buf, 0);
        if (n == 0 || (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) { c.dead = true; return; }
        if (n < 0 || c.sse) return;
        c.in.append(buf, static_cast<size_t>(n));
        if (c.in.size() > kMaxRequest) { c.dead = true; return; }
        if (c.in.find("\r\n\r\n") != std::string::npos) route(c);
    }

    void write_client(Client& c) {
        while (pending(c)) {
            ssize_t n = ::send(c.fd, c.out.data() + c.sent, c.out.size() - c.sent, 0);
            if (n < 0) { if (errno != EAGAIN && errno != EWOULDBLOCK) c.dead = true; return; }
            c.sent += static_cast<size_t>(n);
        }
        c.out.clear(); c.sent = 0;
        if (c.closing) c.dead = true;
    }

    void respond(Client& c, int code, const char* type, const std::string& body) {
        c.out += "HTTP/1.1 " + std::to_string(code) + (code == 200 ? " OK" : " Error") + "\r\nContent-Type: " + type + "\r\nCache-Control: no-store\r\nConnection: close\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
        c.closing = true;
        write_client(c);
    }

    // Other sites in the browser must not be able to reach this server.
    bool allowed(const std::string& req) const {
        const std::string host = header(req, "host"), port = std::to_string(opt_.http_port);
        if (host != "127.0.0.1:" + port && host != "localhost:" + port) return false;
        const std::string origin = header(req, "origin");
        return origin.empty() || origin == "http://" + host;
    }

    void route(Client& c) {
        const size_t sp1 = c.in.find(' '), sp2 = c.in.find(' ', sp1 + 1);
        if (sp1 == std::string::npos || sp2 == std::string::npos) { c.dead = true; return; }
        if (!allowed(c.in)) { respond(c, 403, "text/plain", "forbidden"); return; }
        const std::string method = c.in.substr(0, sp1), target = c.in.substr(sp1 + 1, sp2 - sp1 - 1);
        const size_t q = target.find('?');
        const std::string path = target.substr(0, q);
        std::string sym;
        if (q != std::string::npos && target.compare(q + 1, 2, "s=") == 0) sym = target.substr(q + 3);
        std::transform(sym.begin(), sym.end(), sym.begin(), [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });
        const int64_t now = ms_now();

        if (method == "GET" && path == "/") {
            std::ifstream f(opt_.web);
            if (!f) { respond(c, 500, "text/plain", "cannot read " + opt_.web); return; }
            std::stringstream ss; ss << f.rdbuf();
            respond(c, 200, "text/html; charset=utf-8", ss.str());
        } else if (method == "GET" && path == "/events") {
            c.sse = true;
            c.out += "HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\nCache-Control: no-store\r\nConnection: keep-alive\r\n\r\n";
            std::string o = "{\"k\":\"rows\",\"full\":1,\"rows\":[";
            for (auto& s : slots_) if (s->active) { row(o, *s, now); o += ','; }
            close_obj(o, ']');
            o += '}';
            c.out += "data: " + o + "\n\ndata: " + status(now) + "\n\n";
            write_client(c);
        } else if (method == "GET" && path == "/api/detail") {
            auto it = index_.find(sym);
            if (it == index_.end() || !slots_[static_cast<size_t>(it->second)]->active) { respond(c, 404, "application/json", "{}"); return; }
            respond(c, 200, "application/json", detail(*slots_[static_cast<size_t>(it->second)]));
        } else if (method == "POST" && path == "/api/add") {
            add(c, sym, now);
        } else if (method == "POST" && path == "/api/remove") {
            remove(c, sym);
        } else {
            respond(c, 404, "text/plain", "not found");
        }
    }

    std::string detail(Slot& s) {
        std::string o = "{";
        put_str(o, "s", s.symbol);
        o += "\"bars\":[";
        for (const auto& b : s.book.bars()) {
            o += '['; o += std::to_string(b.ms); o += ',';
            num(o, b.close); num(o, b.bid); num(o, b.ask); num(o, b.vol); num(o, b.buy); num(o, b.sell);
            close_obj(o, ']'); o += ',';
        }
        close_obj(o, ']'); o += ",\"tape\":[";
        const auto& t = s.book.trades();
        for (size_t i = t.size() > 150 ? t.size() - 150 : 0; i < t.size(); ++i) {
            o += '['; o += std::to_string(t[i].ms); o += ',';
            num(o, t[i].px); num(o, t[i].sz); o += std::to_string(t[i].side); o += "],";
        }
        close_obj(o, ']'); o += ",\"daily\":[";
        for (size_t i = s.daily.size() > 130 ? s.daily.size() - 130 : 0; i < s.daily.size(); ++i) {
            o += '['; escape(o, s.dates[i]); o += ','; num(o, s.daily[i].close); num(o, s.daily[i].volume); close_obj(o, ']'); o += ',';
        }
        close_obj(o, ']');
        o += '}';
        return o;
    }

    void add(Client& c, const std::string& sym, int64_t now) {
        if (!valid_symbol(sym)) { respond(c, 400, "text/plain", "bad symbol"); return; }
        auto it = index_.find(sym);
        if (it != index_.end() && slots_[static_cast<size_t>(it->second)]->active) { respond(c, 200, "text/plain", "ok"); return; }
        if (active_lines() >= opt_.max_lines) { respond(c, 409, "text/plain", "line limit reached"); return; }
        if (slots_.size() >= static_cast<size_t>(kHistBase)) { respond(c, 409, "text/plain", "restart to reuse slots"); return; }
        int id;
        if (it != index_.end()) { id = it->second; auto& s = *slots_[static_cast<size_t>(id)]; s.active = true; s.book = Book(); s.book.start(now); s.error = 0; s.error_text.clear(); s.hist_done = false; s.hist_status = "PENDING"; }
        else id = add_slot(sym, now);
        if (gw_fd_ >= 0 && ready_) enqueue(id);
        save_watchlist();
        respond(c, 200, "text/plain", "ok");
    }

    void remove(Client& c, const std::string& sym) {
        auto it = index_.find(sym);
        if (it == index_.end() || !slots_[static_cast<size_t>(it->second)]->active) { respond(c, 404, "text/plain", "not on watchlist"); return; }
        auto& s = *slots_[static_cast<size_t>(it->second)];
        s.active = false; s.dirty = false;
        try { if (gw_fd_ >= 0 && ready_) send_all(gw_fd_, cancel_quotes(it->second)); } catch (const std::exception& e) { drop_gateway(e.what()); }
        save_watchlist();
        std::string o = "{\"k\":\"removed\",";
        put_str(o, "s", sym);
        close_obj(o, '}');
        broadcast(o);
        respond(c, 200, "text/plain", "ok");
    }

    Options opt_;
    int listen_fd_ = -1, gw_fd_ = -1;
    Framer framer_;
    bool ready_ = false, any_dirty_ = false;
    std::vector<std::unique_ptr<Slot>> slots_;
    std::map<std::string, int> index_;
    std::vector<std::unique_ptr<Client>> clients_;
    std::deque<int> sub_queue_, hist_queue_;
    int64_t last_sub_ = 0, last_hist_ = 0, last_flush_ = 0, last_connect_ = 0;
    int64_t messages_ = 0, last_messages_ = 0;
    std::vector<int64_t> engine_ns_, wire_ns_;
    std::string account_kind_ = "?", gw_error_, last_error_text_;
    Session session_{"?", 0};
};

}  // namespace

int main(int argc, char** argv) {
    Options o;
    for (int i = 1; i + 1 < argc; i += 2) {
        const std::string k = argv[i], v = argv[i + 1];
        if (k == "--gateway-port") o.gw_port = std::stoi(v);
        else if (k == "--port") o.http_port = std::stoi(v);
        else if (k == "--client-id") o.client_id = std::stoi(v);
        else if (k == "--flush-ms") o.flush_ms = std::stoi(v);
        else if (k == "--max-lines") o.max_lines = std::stoi(v);
        else if (k == "--ticket") o.ticket = std::stod(v);
        else if (k == "--watchlist") o.watch = v;
        else if (k == "--web") o.web = v;
        else if (k == "--mode") o.mode = v == "realtime" ? 1 : v == "frozen" ? 2 : v == "delayed" ? 3 : v == "delayed-frozen" ? 4 : 0;
        else { std::cerr << "unknown option " << k << "\n"; return 2; }
    }
    if (o.mode == 0) { std::cerr << "bad --mode\n"; return 2; }
    setenv("TZ", "America/New_York", 1);
    tzset();
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);
    std::signal(SIGPIPE, SIG_IGN);
    try { return Dash(o).run(); }
    catch (const std::exception& e) { std::cerr << "dash error: " << e.what() << "\n"; return 1; }
}
