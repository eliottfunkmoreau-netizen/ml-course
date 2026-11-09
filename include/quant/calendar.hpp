#pragma once
// Turn-of-month test. Day -1 is a month's last trading day; the rule buys at the
// close of Day -2 and sells at the close of Day +3, capturing Days -1,+1,+2,+3.
#include <algorithm>
#include <cmath>
#include <map>
#include <random>
#include <vector>

#include "gates.hpp"

namespace quant {

struct CalendarCost {
    double notional_usd = 2460, commission_usd = 0.35, half_spread_bps = 1;
    double per_order() const { return commission_usd / notional_usd + half_spread_bps * 1e-4; }
};

// Marks each row belonging to a complete window whose first day is on or after `from`.
inline std::vector<int> window_flags(const std::vector<int>& dates, int from, int to) {
    std::vector<int> w(dates.size(), 0);
    for (size_t t = 1; t + 3 < dates.size(); ++t) {
        if (dates[t] / 100 == dates[t + 1] / 100 || dates[t] < from || dates[t + 3] > to) continue;
        for (size_t k = t; k <= t + 3; ++k) w[k] = 1;
    }
    return w;
}

// Long side holds window days. Invert holds the complete gaps between windows.
inline std::vector<Day> calendar_days(const std::vector<int>& dates, const std::vector<double>& adj, int from, int to,
                                      const CalendarCost& c, bool invert) {
    const auto w = window_flags(dates, from, to);
    std::vector<int> pos(dates.size(), 0);
    std::vector<size_t> starts;
    for (size_t t = 1; t < dates.size(); ++t) if (w[t] && !w[t - 1]) starts.push_back(t);
    if (!invert) pos = w;
    else
        for (size_t s = 0; s + 1 < starts.size(); ++s)
            for (size_t k = starts[s] + 4; k < starts[s + 1]; ++k) pos[k] = 1;
    std::vector<Day> out;
    for (size_t t = 1; t < dates.size(); ++t) {
        if (dates[t] < from || dates[t] > to) continue;
        Day d;
        d.date = dates[t];
        d.bench = adj[t] / adj[t - 1] - 1;
        d.exposure = pos[t];
        d.gross = pos[t] ? d.bench : 0.0;
        if (pos[t] && !pos[t - 1]) { d.cost += c.per_order(); ++d.tickets; }
        if (pos[t] && (t + 1 >= dates.size() || !pos[t + 1])) { d.cost += c.per_order(); ++d.tickets; }
        out.push_back(d);
    }
    return out;
}

// Lower `alpha` quantile of the mean per-cluster sum, resampling whole clusters.
template <class KeyFn>
double cluster_bootstrap_lb(const std::vector<std::pair<int, double>>& daily, KeyFn key, double alpha, uint64_t seed, int draws) {
    std::map<long, double> by;
    for (auto& [date, v] : daily) by[key(date)] += v;
    std::vector<double> xs;
    for (auto& [k, v] : by) xs.push_back(v);
    if (xs.size() < 2) return -1;
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<size_t> pick(0, xs.size() - 1);
    std::vector<double> means(static_cast<size_t>(draws));
    for (auto& m : means) {
        double s = 0;
        for (size_t i = 0; i < xs.size(); ++i) s += xs[pick(rng)];
        m = s / static_cast<double>(xs.size());
    }
    std::sort(means.begin(), means.end());
    return means[std::min(means.size() - 1, static_cast<size_t>(alpha * static_cast<double>(means.size())))];
}

inline long month_key(int yyyymmdd) { return yyyymmdd / 100; }

struct AdaptedResult {
    std::vector<std::pair<std::string, bool>> gates;
    double net_daily = 0, elevated_daily = 0, active_daily = 0, weekly_lb = 0, monthly_lb = 0;
    double window_mean = 0, other_mean = 0, difference_t = 0, opp_active_daily = 0, long_minus_opp_weekly_lb = 0, worst_year = 0;
    bool passed() const { return !gates.empty() && std::all_of(gates.begin(), gates.end(), [](auto& g) { return g.second; }); }
};

inline AdaptedResult evaluate_adapted(const std::vector<Day>& d, const std::vector<Day>& opp, double exposure, double alpha, double mult, uint64_t seed, int draws) {
    AdaptedResult r;
    std::vector<std::pair<int, double>> active, opp_active, diff;
    std::map<int, double> years;
    double net = 0, elev = 0, w = 0, o = 0, w2 = 0, o2 = 0, oa = 0;
    size_t nw = 0, no = 0;
    for (size_t i = 0; i < d.size(); ++i) {
        const double n = d[i].gross - d[i].cost, a = n - exposure * d[i].bench;
        net += n; elev += d[i].gross - d[i].cost * mult;
        active.emplace_back(d[i].date, a);
        years[d[i].date / 10000] += n;
        if (d[i].exposure > 0) { w += d[i].bench; w2 += d[i].bench * d[i].bench; ++nw; }
        else { o += d[i].bench; o2 += d[i].bench * d[i].bench; ++no; }
    }
    for (size_t i = 0; i < d.size() && i < opp.size(); ++i) {
        const double n = opp[i].gross - opp[i].cost, a = n - (1 - exposure) * opp[i].bench;
        oa += a;
        opp_active.emplace_back(opp[i].date, a);
        diff.emplace_back(d[i].date, (d[i].gross - d[i].cost) - n);
    }
    const double T = static_cast<double>(d.size());
    r.net_daily = net / T; r.elevated_daily = elev / T;
    for (auto& a : active) r.active_daily += a.second / T;
    r.opp_active_daily = oa / T;
    r.window_mean = nw ? w / static_cast<double>(nw) : 0;
    r.other_mean = no ? o / static_cast<double>(no) : 0;
    if (nw > 1 && no > 1) {
        const double vw = w2 / static_cast<double>(nw) - r.window_mean * r.window_mean, vo = o2 / static_cast<double>(no) - r.other_mean * r.other_mean;
        r.difference_t = (r.window_mean - r.other_mean) / std::sqrt(vw / static_cast<double>(nw) + vo / static_cast<double>(no));
    }
    auto week = [](int x) { return week_key(x); };
    r.weekly_lb = cluster_bootstrap_lb(active, week, alpha, seed, draws);
    r.monthly_lb = cluster_bootstrap_lb(active, month_key, alpha, seed + 1, draws);
    const double opp_lb = cluster_bootstrap_lb(opp_active, week, alpha, seed + 2, draws);
    r.long_minus_opp_weekly_lb = cluster_bootstrap_lb(diff, week, alpha, seed + 3, draws);
    bool no_neg = true;
    r.worst_year = 1e9;
    for (auto& [y, v] : years) { r.worst_year = std::min(r.worst_year, v); no_neg = no_neg && v > 0; }
    r.gates = {
        {"net_positive", r.net_daily > 0},
        {"positive_at_doubled_costs", r.elevated_daily > 0},
        {"weekly_bootstrap_active_lb_positive", r.weekly_lb > 0},
        {"monthly_bootstrap_active_lb_positive", r.monthly_lb > 0},
        {"no_negative_absolute_year", no_neg},
        {"beats_exposure_matched_benchmark", r.active_daily > 0},
        {"window_beats_other_days", r.window_mean > r.other_mean},
        {"opposite_fails_active_test", !(r.opp_active_daily > 0 && opp_lb > 0)},
        {"long_minus_opposite_lb_positive", r.long_minus_opp_weekly_lb > 0},
    };
    return r;
}

// Inverse standard normal CDF by bisection on erfc.
inline double inv_norm(double p) {
    double lo = -10, hi = 10;
    for (int i = 0; i < 200; ++i) {
        double m = (lo + hi) / 2;
        if (0.5 * std::erfc(-m / std::sqrt(2.0)) < p) lo = m; else hi = m;
    }
    return (lo + hi) / 2;
}

// Deflated Sharpe on per-period values with N trials, assuming null Sharpe variance 1/T.
inline double deflated_sharpe(const std::vector<double>& x, int trials) {
    const double T = static_cast<double>(x.size());
    double m = 0, m2 = 0, m3 = 0, m4 = 0;
    for (double v : x) m += v / T;
    for (double v : x) { double d = v - m; m2 += d * d / T; m3 += d * d * d / T; m4 += d * d * d * d / T; }
    if (m2 <= 0) return 0;
    const double sr = m / std::sqrt(m2), g3 = m3 / std::pow(m2, 1.5), g4 = m4 / (m2 * m2);
    const double euler = 0.5772156649, e = std::exp(1.0), n = static_cast<double>(trials);
    const double sr0 = std::sqrt(1.0 / T) * ((1 - euler) * inv_norm(1 - 1 / n) + euler * inv_norm(1 - 1 / (n * e)));
    const double z = (sr - sr0) * std::sqrt(T - 1) / std::sqrt(1 - g3 * sr + (g4 - 1) / 4 * sr * sr);
    return 0.5 * std::erfc(-z / std::sqrt(2.0));
}

}  // namespace quant
