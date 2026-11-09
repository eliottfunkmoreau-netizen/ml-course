#pragma once
// Minimal IBKR socket protocol (text framing, server version 187, below the
// protobuf cutover at 201). Outbound messages are restricted by type to market
// data requests: this client has no way to express an order.
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace quant::ib {

inline constexpr int kServerVersion = 187;
inline constexpr size_t kMaxFrame = 1 << 24;

enum class Out : int { ReqMktData = 1, CancelMktData = 2, ReqHistoricalData = 20, ReqMarketDataType = 59, StartApi = 71 };

inline std::string be32(uint32_t n) {
    return {static_cast<char>(n >> 24), static_cast<char>(n >> 16), static_cast<char>(n >> 8), static_cast<char>(n)};
}

inline std::string handshake() {
    const std::string v = "v100.." + std::to_string(kServerVersion);
    return std::string("API\0", 4) + be32(static_cast<uint32_t>(v.size())) + v;
}

inline std::string frame(Out id, const std::vector<std::string>& fields) {
    std::string body = std::to_string(static_cast<int>(id));
    body.push_back('\0');
    for (auto& f : fields) { body += f; body.push_back('\0'); }
    return be32(static_cast<uint32_t>(body.size())) + body;
}

inline std::string start_api(int client_id) { return frame(Out::StartApi, {"2", std::to_string(client_id), ""}); }

// 1 realtime, 2 frozen, 3 delayed, 4 delayed-frozen.
inline std::string req_market_data_type(int mode) { return frame(Out::ReqMarketDataType, {"1", std::to_string(mode)}); }

inline std::string req_stock_quotes(int req_id, const std::string& symbol) {
    return frame(Out::ReqMktData, {"11", std::to_string(req_id), "0", symbol, "STK", "", "", "", "", "SMART", "", "USD", "", "", "0", "", "0", "0", ""});
}

// As req_stock_quotes, plus generic tick 233 (RTVolume), which carries each trade's
// price, size, time and the day's VWAP.
inline std::string req_quotes_with_trades(int req_id, const std::string& symbol) {
    return frame(Out::ReqMktData, {"11", std::to_string(req_id), "0", symbol, "STK", "", "", "", "", "SMART", "", "USD", "", "", "0", "233", "0", "0", ""});
}

// Split-adjusted, regular-hours daily TRADES bars ending now. Dates arrive as yyyyMMdd.
inline std::string req_daily_history(int req_id, const std::string& symbol, const std::string& duration) {
    return frame(Out::ReqHistoricalData, {std::to_string(req_id), "0", symbol, "STK", "", "", "", "", "SMART", "", "USD", "", "", "0", "", "1 day", duration, "1", "TRADES", "1", "0", ""});
}

inline std::string cancel_quotes(int req_id) { return frame(Out::CancelMktData, {"2", std::to_string(req_id)}); }

// Reassembles length-prefixed frames from an arbitrary byte stream.
class Framer {
public:
    void feed(const char* data, size_t n) { buf_.append(data, n); }
    bool next(std::string& out) {
        if (buf_.size() - pos_ < 4) { compact(); return false; }
        auto b = [&](size_t i) { return static_cast<uint32_t>(static_cast<unsigned char>(buf_[pos_ + i])); };
        const uint32_t len = b(0) << 24 | b(1) << 16 | b(2) << 8 | b(3);
        if (len > kMaxFrame) throw std::runtime_error("ib: oversized frame");
        if (buf_.size() - pos_ - 4 < len) { compact(); return false; }
        out.assign(buf_, pos_ + 4, len);
        pos_ += 4 + len;
        return true;
    }

private:
    void compact() { if (pos_) { buf_.erase(0, pos_); pos_ = 0; } }
    std::string buf_;
    size_t pos_ = 0;
};

inline std::vector<std::string_view> split_fields(std::string_view body) {
    std::vector<std::string_view> out;
    size_t start = 0;
    for (size_t i = 0; i < body.size(); ++i)
        if (body[i] == '\0') { out.push_back(body.substr(start, i - start)); start = i + 1; }
    return out;
}

enum class Kind { Other, Price, Size, Error, NextValidId, ManagedAccounts, MarketDataType, String, Generic };

struct Event {
    Kind kind = Kind::Other;
    int id = -1, tick = -1, code = 0, data_type = 0;
    double value = 0;
    std::string text;
};

inline int to_int(std::string_view s) { return s.empty() ? 0 : std::stoi(std::string(s)); }
inline double to_double(std::string_view s) { return s.empty() ? 0.0 : std::stod(std::string(s)); }

inline Event decode(const std::vector<std::string_view>& f) {
    Event e;
    if (f.empty()) return e;
    const int msg = to_int(f[0]);
    if (msg == 1 && f.size() >= 7) { e.kind = Kind::Price; e.id = to_int(f[2]); e.tick = to_int(f[3]); e.value = to_double(f[4]); }
    else if (msg == 2 && f.size() >= 5) { e.kind = Kind::Size; e.id = to_int(f[2]); e.tick = to_int(f[3]); e.value = to_double(f[4]); }
    else if (msg == 4 && f.size() >= 5) { e.kind = Kind::Error; e.id = to_int(f[2]); e.code = to_int(f[3]); e.text = std::string(f[4]); }
    else if (msg == 9 && f.size() >= 3) { e.kind = Kind::NextValidId; e.id = to_int(f[2]); }
    else if (msg == 15 && f.size() >= 3) { e.kind = Kind::ManagedAccounts; e.text = std::string(f[2]); }
    else if (msg == 58 && f.size() >= 4) { e.kind = Kind::MarketDataType; e.id = to_int(f[2]); e.data_type = to_int(f[3]); }
    else if (msg == 45 && f.size() >= 5) { e.kind = Kind::Generic; e.id = to_int(f[2]); e.tick = to_int(f[3]); e.value = to_double(f[4]); }
    else if (msg == 46 && f.size() >= 5) { e.kind = Kind::String; e.id = to_int(f[2]); e.tick = to_int(f[3]); e.text = std::string(f[4]); }
    return e;
}

}  // namespace quant::ib
