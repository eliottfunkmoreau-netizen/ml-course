#pragma once
// Per-symbol OTC analytics from Level 1 quotes and the RTVolume trade stream.
// Pure and single-threaded: every input carries its clock, so a test can replay a session.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

namespace quant::otc {

inline constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();
inline constexpr int64_t kBarMs = 5000;
inline constexpr size_t kMaxBars = 1440, kMaxTrades = 2000, kMaxSamples = 121;

struct Trade { int64_t ms = 0; double px = 0, sz = 0; int side = 0; };
struct Bar { int64_t ms = 0; double open = kNaN, high = kNaN, low = kNaN, close = kNaN, bid = kNaN, ask = kNaN, vol = 0, buy = 0, sell = 0; };
struct Daily { double open = 0, high = 0, low = 0, close = 0, volume = 0; };

struct DailyStats {
    bool ok = false;
    double adv20 = kNaN, dollar_adv20 = kNaN, atr14_pct = kNaN, rv20_ann_pct = kNaN, high_1y = kNaN, low_1y = kNaN;
    int zero_days60 = 0;
};

inline DailyStats daily_stats(const std::vector<Daily>& d) {
    DailyStats s;
    if (d.size() < 21) return s;
    const size_t n = d.size();
    double v = 0, dv = 0;
    for (size_t i = n - 20; i < n; ++i) { v += d[i].volume; dv += d[i].volume * d[i].close; }
    s.adv20 = v / 20; s.dollar_adv20 = dv / 20;
    double atr = 0;
    for (size_t i = n - 14; i < n; ++i)
        atr += std::max({d[i].high - d[i].low, std::abs(d[i].high - d[i - 1].close), std::abs(d[i].low - d[i - 1].close)});
    if (d[n - 1].close > 0) s.atr14_pct = atr / 14 / d[n - 1].close * 100;
    double mean = 0, var = 0;
    std::array<double, 20> r{};
    for (size_t i = 0; i < 20; ++i) { const auto& a = d[n - 21 + i]; const auto& b = d[n - 20 + i]; r[i] = a.close > 0 && b.close > 0 ? std::log(b.close / a.close) : 0; mean += r[i]; }
    mean /= 20;
    for (double x : r) var += (x - mean) * (x - mean);
    s.rv20_ann_pct = std::sqrt(var / 19 * 252) * 100;
    s.high_1y = d[0].high; s.low_1y = d[0].low;
    for (const auto& b : d) { s.high_1y = std::max(s.high_1y, b.high); if (b.low > 0) s.low_1y = std::min(s.low_1y, b.low); }
    for (size_t i = n >= 60 ? n - 60 : 0; i < n; ++i) s.zero_days60 += d[i].volume <= 0;
    s.ok = true;
    return s;
}

// IBKR tiered US stock commission, one side.
inline double commission(double shares, double notional) { return std::min(0.01 * notional, std::max(0.35, 0.0035 * shares)); }

struct Flow { double buy = 0, sell = 0, total = 0; int trades = 0; };

struct Metrics {
    double bid = kNaN, ask = kNaN, bid_sz = kNaN, ask_sz = kNaN, last = kNaN, last_sz = kNaN, open = kNaN, high = kNaN, low = kNaN, close = kNaN, volume = kNaN, vwap = kNaN;
    double mid = kNaN, spread = kNaN, spread_pct = kNaN, micro = kNaN, imbalance = kNaN, change_pct = kNaN, vwap_dev_pct = kNaN, dollar_vol = kNaN;
    double bid_notional = kNaN, ask_notional = kNaN, tw_spread_pct = kNaN, two_sided_pct = kNaN, eff_spread_pct = kNaN, rv_1m_bps = kNaN, mid_5m_pct = kNaN;
    double cost_pct = kNaN, fill_pct = kNaN, pace = kNaN, range_pos = kNaN;
    Flow flow_1m, flow_5m, flow_15m;
    int quotes_per_min = 0, halted = 0;
    int64_t last_quote_ms = 0, last_trade_ms = 0;
    char grade = '-';
    std::vector<std::string> flags;
};

class Book {
public:
    void start(int64_t now) { start_ms_ = now; }
    void set_daily(DailyStats s) { daily_ = s; }
    const DailyStats& daily() const { return daily_; }
    const std::deque<Bar>& bars() const { return bars_; }
    const std::deque<Trade>& trades() const { return trades_; }

    void on_price(int tick, double v, int64_t now) {
        switch (normal(tick)) {
            case 1: accrue(now); bid_ = v > 0 ? v : kNaN; touch_quote(now); break;
            case 2: accrue(now); ask_ = v > 0 ? v : kNaN; touch_quote(now); break;
            case 4: last_ = v > 0 ? v : last_; break;
            case 6: high_ = v > 0 ? v : kNaN; break;
            case 7: low_ = v > 0 ? v : kNaN; break;
            case 9: close_ = v > 0 ? v : kNaN; break;
            case 14: open_ = v > 0 ? v : kNaN; break;
            default: break;
        }
    }

    void on_size(int tick, double v, int64_t now) {
        switch (normal(tick)) {
            case 0: bid_sz_ = v; touch_quote(now); break;
            case 3: ask_sz_ = v; touch_quote(now); break;
            case 5:
                last_sz_ = v;
                // Delayed data has no RTVolume, so its last-size ticks stand in for prints. The first
                // one after subscribing is a snapshot of an old trade, not a new one.
                if (tick == 71 && !rt_seen_ && snapshot_seen_ && v > 0 && std::isfinite(last_)) add_trade(now, last_, v, now);
                snapshot_seen_ = true;
                break;
            case 8: volume_ = v; break;
            default: break;
        }
    }

    // Format: price;size;epochMs;dayVolume;dayVwap;singleTradeFlag. Price and size are empty on volume corrections.
    void on_rt_volume(std::string_view s, int64_t now) {
        std::array<std::string_view, 6> f{};
        size_t n = 0, a = 0;
        for (size_t i = 0; i <= s.size() && n < f.size(); ++i)
            if (i == s.size() || s[i] == ';') { f[n++] = s.substr(a, i - a); a = i + 1; }
        if (n < 5) return;
        rt_seen_ = true;
        const double vol = num(f[3]), vw = num(f[4]);
        if (vol > 0) volume_ = vol;
        if (vw > 0) vwap_ = vw;
        const double px = num(f[0]), sz = num(f[1]);
        const int64_t ms = static_cast<int64_t>(num(f[2]));
        if (!(px > 0) || !(sz > 0)) return;
        last_ = px; last_sz_ = sz; last_trade_ms_ = ms;
        // Gateway replays the most recent print on subscribe. It belongs to no window we measure.
        if (ms >= start_ms_ - 2000) add_trade(ms, px, sz, now);
    }

    void on_halt(double v) { halted_ = static_cast<int>(v); }

    // Called about once a second. Samples the mid on a fixed grid for volatility.
    void clock(int64_t now) {
        if (now < next_sample_) return;
        next_sample_ = now - now % 60000 + 60000;
        const double m = mid();
        if (std::isfinite(m)) { samples_.push_back({now, m}); if (samples_.size() > kMaxSamples) samples_.pop_front(); }
    }

    Metrics metrics(int64_t now, double ticket, double session_fraction) {
        accrue(now);
        Metrics m;
        m.bid = bid_; m.ask = ask_; m.bid_sz = bid_sz_; m.ask_sz = ask_sz_; m.last = last_; m.last_sz = last_sz_; m.open = open_; m.high = high_; m.low = low_; m.close = close_; m.volume = volume_; m.vwap = vwap_;
        m.last_quote_ms = last_quote_ms_; m.last_trade_ms = last_trade_ms_; m.halted = halted_;
        const bool two = std::isfinite(bid_) && std::isfinite(ask_);
        if (two) {
            m.mid = (bid_ + ask_) / 2; m.spread = ask_ - bid_; m.spread_pct = m.spread / m.mid * 100;
            if (bid_sz_ + ask_sz_ > 0) {
                m.micro = (bid_ * ask_sz_ + ask_ * bid_sz_) / (bid_sz_ + ask_sz_);
                m.imbalance = (bid_sz_ - ask_sz_) / (bid_sz_ + ask_sz_);
            }
            const double shares = ticket / ask_, comm = commission(shares, ticket);
            m.cost_pct = (m.spread * shares + 2 * comm) / ticket * 100;
            if (ask_sz_ > 0) m.fill_pct = std::min(1.0, ask_sz_ * ask_ / ticket) * 100;
        }
        if (bid_ > 0 && bid_sz_ >= 0) m.bid_notional = bid_ * bid_sz_;
        if (ask_ > 0 && ask_sz_ >= 0) m.ask_notional = ask_ * ask_sz_;
        if (last_ > 0 && close_ > 0) m.change_pct = (last_ / close_ - 1) * 100;
        if (last_ > 0 && vwap_ > 0) m.vwap_dev_pct = (last_ / vwap_ - 1) * 100;
        if (volume_ >= 0 && vwap_ > 0) m.dollar_vol = volume_ * vwap_;
        else if (volume_ >= 0 && last_ > 0) m.dollar_vol = volume_ * last_;
        if (time_ms_ > 0) {
            m.two_sided_pct = two_sided_ms_ / time_ms_ * 100;
            if (two_sided_ms_ > 0) m.tw_spread_pct = spread_ms_ / two_sided_ms_;
        }
        if (eff_n_ > 0) m.eff_spread_pct = eff_sum_ / eff_n_;
        for (auto it = trades_.rbegin(); it != trades_.rend() && it->ms >= now - 900000; ++it) {
            add_flow(m.flow_15m, *it);
            if (it->ms >= now - 300000) add_flow(m.flow_5m, *it);
            if (it->ms >= now - 60000) add_flow(m.flow_1m, *it);
        }
        for (size_t i = 0; i < quote_counts_.size(); ++i) if (quote_secs_[i] > now / 1000 - 60) m.quotes_per_min += quote_counts_[i];
        volatility(m, now);
        if (daily_.ok) {
            if (daily_.adv20 > 0 && volume_ >= 0 && session_fraction > 0) m.pace = volume_ / (daily_.adv20 * session_fraction);
            const double hi = std::max(daily_.high_1y, last_), lo = std::min(daily_.low_1y, last_);
            if (last_ > 0 && hi > lo) m.range_pos = (last_ - lo) / (hi - lo) * 100;
        }
        grade(m, two, ticket);
        return m;
    }

private:
    // Delayed tick ids map onto their realtime twins.
    static int normal(int t) {
        switch (t) { case 66: return 1; case 67: return 2; case 68: return 4; case 69: return 0; case 70: return 3; case 71: return 5; case 72: return 6; case 73: return 7; case 74: return 8; case 75: return 9; case 76: return 14; default: return t; }
    }
    static double num(std::string_view s) { if (s.empty()) return 0; return std::strtod(std::string(s).c_str(), nullptr); }
    double mid() const { return std::isfinite(bid_) && std::isfinite(ask_) ? (bid_ + ask_) / 2 : kNaN; }

    void accrue(int64_t now) {
        if (accrued_ms_ && now > accrued_ms_) {
            const double dt = static_cast<double>(now - accrued_ms_);
            time_ms_ += dt;
            if (std::isfinite(bid_) && std::isfinite(ask_) && ask_ >= bid_) { two_sided_ms_ += dt; spread_ms_ += dt * (ask_ - bid_) / mid() * 100; }
        }
        accrued_ms_ = now;
    }

    void touch_quote(int64_t now) {
        last_quote_ms_ = now;
        const size_t i = static_cast<size_t>(now / 1000 % 60);
        if (quote_secs_[i] != now / 1000) { quote_secs_[i] = now / 1000; quote_counts_[i] = 0; }
        ++quote_counts_[i];
        Bar& b = bar(now);
        b.bid = bid_; b.ask = ask_;
    }

    Bar& bar(int64_t now) {
        const int64_t t = now - now % kBarMs;
        if (bars_.empty() || bars_.back().ms != t) {
            Bar b; b.ms = t; b.bid = bid_; b.ask = ask_;
            bars_.push_back(b);
            if (bars_.size() > kMaxBars) bars_.pop_front();
        }
        return bars_.back();
    }

    void add_trade(int64_t ms, double px, double sz, int64_t now) {
        // Lee-Ready: quote rule first, tick test when the print is exactly at the mid or no quote exists.
        int side = 0;
        const double m = mid();
        if (std::isfinite(m)) {
            if (px >= ask_) side = 1; else if (px <= bid_) side = -1; else if (px > m) side = 1; else if (px < m) side = -1;
            eff_sum_ += 2 * std::abs(px - m) / m * 100; ++eff_n_;
        }
        if (side == 0 && prev_px_ > 0) side = px > prev_px_ ? 1 : px < prev_px_ ? -1 : prev_side_;
        prev_px_ = px; prev_side_ = side;
        trades_.push_back({ms, px, sz, side});
        if (trades_.size() > kMaxTrades) trades_.pop_front();
        last_trade_ms_ = ms;
        Bar& b = bar(now);
        if (!std::isfinite(b.open)) b.open = b.high = b.low = px;
        b.high = std::max(b.high, px); b.low = std::min(b.low, px); b.close = px; b.vol += sz;
        if (side > 0) b.buy += sz; else if (side < 0) b.sell += sz;
    }

    static void add_flow(Flow& f, const Trade& t) {
        f.total += t.sz; ++f.trades;
        if (t.side > 0) f.buy += t.sz; else if (t.side < 0) f.sell += t.sz;
    }

    void volatility(Metrics& m, int64_t now) const {
        if (samples_.size() >= 6) {
            double mean = 0, var = 0;
            const size_t n = samples_.size() - 1;
            for (size_t i = 1; i <= n; ++i) mean += std::log(samples_[i].second / samples_[i - 1].second);
            mean /= static_cast<double>(n);
            for (size_t i = 1; i <= n; ++i) { const double x = std::log(samples_[i].second / samples_[i - 1].second) - mean; var += x * x; }
            m.rv_1m_bps = std::sqrt(var / static_cast<double>(n - 1)) * 1e4;
        }
        for (auto it = samples_.rbegin(); it != samples_.rend(); ++it)
            if (it->first <= now - 300000) { if (std::isfinite(m.mid)) m.mid_5m_pct = (m.mid / it->second - 1) * 100; break; }
    }

    void grade(Metrics& m, bool two, double ticket) const {
        auto flag = [&](const char* s) { m.flags.emplace_back(s); };
        if (m.halted) flag("HALT");
        if (!two) flag(std::isfinite(bid_) || std::isfinite(ask_) ? "ONE-SIDED" : "NO-QUOTE");
        else if (bid_ >= ask_) flag(bid_ > ask_ ? "CROSSED" : "LOCKED");
        // A lone quote far from the last print is a placeholder, not a price anyone expects to trade at.
        if (!two && last_ > 0 && ((std::isfinite(ask_) && ask_ > 2 * last_) || (std::isfinite(bid_) && bid_ < last_ / 2))) flag("STUB");
        if (two && m.spread_pct > 5) flag("WIDE");
        if (last_ > 0 && last_ < 1) flag("SUB$1");
        if (two && std::min(m.bid_notional, m.ask_notional) < ticket) flag("THIN");
        if (daily_.ok && daily_.zero_days60 >= 6) flag("GAPPY");
        if (std::isfinite(m.pace) && m.pace >= 3) flag("VOL-SURGE");
        if (!two || bid_ >= ask_) { m.grade = 'F'; return; }
        int score = m.spread_pct < 1 ? 3 : m.spread_pct < 3 ? 2 : m.spread_pct < 6 ? 1 : 0;
        if (std::isfinite(daily_.dollar_adv20)) score += daily_.dollar_adv20 >= 1e6 ? 3 : daily_.dollar_adv20 >= 1e5 ? 2 : daily_.dollar_adv20 >= 1e4 ? 1 : 0;
        if (std::min(m.bid_notional, m.ask_notional) >= ticket) score += 1;
        m.grade = score >= 6 ? 'A' : score == 5 ? 'B' : score >= 3 ? 'C' : score >= 1 ? 'D' : 'F';
    }

    double bid_ = kNaN, ask_ = kNaN, bid_sz_ = kNaN, ask_sz_ = kNaN, last_ = kNaN, last_sz_ = kNaN, open_ = kNaN, high_ = kNaN, low_ = kNaN, close_ = kNaN, volume_ = kNaN, vwap_ = kNaN;
    int64_t start_ms_ = 0, last_quote_ms_ = 0, last_trade_ms_ = 0, accrued_ms_ = 0, next_sample_ = 0;
    double time_ms_ = 0, two_sided_ms_ = 0, spread_ms_ = 0, eff_sum_ = 0, prev_px_ = 0;
    long eff_n_ = 0;
    int prev_side_ = 0, halted_ = 0;
    bool rt_seen_ = false, snapshot_seen_ = false;
    DailyStats daily_;
    std::deque<Bar> bars_;
    std::deque<Trade> trades_;
    std::deque<std::pair<int64_t, double>> samples_;
    std::array<int, 60> quote_counts_{};
    std::array<int64_t, 60> quote_secs_{};
};

}  // namespace quant::otc
