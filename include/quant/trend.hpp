#pragma once
// Five-sleeve monthly trend rule. A month-end close is known at that close, and
// trades fill at the next session's open.
#include <cmath>
#include <vector>

#include "valprof.hpp"

namespace quant {

struct TrendData {
    std::vector<int> dates;
    size_t T = 0, N = 0;
    std::vector<double> open, adj;  // Dividend-adjusted, T x N, NaN before listing.
    size_t ix(size_t t, size_t i) const { return t * N + i; }
};

struct TrendParams {
    int from = 0, to = 99999999, sma_months = 10;
    double drift_band = 0.25, notional = 2460, commission = 0.35, half_spread_bps = 3;
};

inline std::vector<size_t> month_end_rows(const std::vector<int>& dates) {
    std::vector<size_t> out;
    for (size_t t = 0; t + 1 < dates.size(); ++t) if (dates[t] / 100 != dates[t + 1] / 100) out.push_back(t);
    return out;
}

// Signal for sleeve i at the m-th month-end: 1 above trend, 0 below, -1 not enough history.
inline int trend_signal(const TrendData& d, const std::vector<size_t>& ends, size_t m, size_t i, int months) {
    if (m + 1 < static_cast<size_t>(months)) return -1;
    double sum = 0;
    for (size_t k = m + 1 - static_cast<size_t>(months); k <= m; ++k) {
        const double x = d.adj[d.ix(ends[k], i)];
        if (!std::isfinite(x)) return -1;
        sum += x;
    }
    return d.adj[d.ix(ends[m], i)] > sum / months ? 1 : 0;
}

inline std::vector<Day> simulate_trend(const TrendData& d, const TrendParams& p, bool invert) {
    const auto ends = month_end_rows(d.dates);
    const double order = p.commission / p.notional, spread = p.half_spread_bps * 1e-4;
    const double target = 1.0 / static_cast<double>(d.N);
    std::vector<double> w(d.N, 0.0), bw(d.N, 0.0);
    std::vector<Day> out;
    size_t m = 0;
    bool started = false;
    int bench_year = 0;
    for (size_t t = 1; t + 1 < d.T && d.dates[t] <= p.to; ++t) {
        while (m < ends.size() && ends[m] + 1 < t) ++m;
        const bool rebalance_day = m < ends.size() && ends[m] + 1 == t;
        if (d.dates[t] < p.from) continue;
        Day day;
        day.date = d.dates[t];
        if (!started || d.dates[t] / 10000 != bench_year) {
            std::fill(bw.begin(), bw.end(), target);
            bench_year = d.dates[t] / 10000;
            started = true;
        }
        if (rebalance_day) {
            double cash = 1.0;
            for (double x : w) cash -= x;
            std::vector<int> want(d.N, -1);
            for (size_t i = 0; i < d.N; ++i) {
                int s = trend_signal(d, ends, m, i, p.sma_months);
                if (s >= 0 && invert) s = 1 - s;
                want[i] = s;
            }
            for (size_t i = 0; i < d.N; ++i) {
                if (want[i] == 0 && w[i] > 0) { cash += w[i]; day.cost += order + w[i] * spread; ++day.tickets; w[i] = 0; }
                else if (want[i] == 1 && w[i] > 0 && std::fabs(w[i] - target) / target > p.drift_band && w[i] > target) {
                    const double x = w[i] - target; cash += x; w[i] = target; day.cost += order + x * spread; ++day.tickets;
                }
            }
            for (size_t i = 0; i < d.N; ++i) {
                if (want[i] != 1) continue;
                double need = 0;
                if (w[i] == 0) need = target;
                else if (w[i] < target && std::fabs(w[i] - target) / target > p.drift_band) need = target - w[i];
                need = std::min(need, std::max(0.0, cash));
                if (need > 1e-12) { w[i] += need; cash -= need; day.cost += order + need * spread; ++day.tickets; }
            }
        }
        double gross = 0, bench = 0, exposure = 0;
        std::vector<double> r(d.N, 0.0);
        for (size_t i = 0; i < d.N; ++i) {
            const double a = d.open[d.ix(t, i)], b = d.open[d.ix(t + 1, i)];
            if (a > 0 && b > 0) r[i] = b / a - 1;
            gross += w[i] * r[i]; bench += bw[i] * r[i]; exposure += w[i];
        }
        for (size_t i = 0; i < d.N; ++i) { w[i] = w[i] * (1 + r[i]) / (1 + gross); bw[i] = bw[i] * (1 + r[i]) / (1 + bench); }
        day.gross = gross; day.bench = bench; day.exposure = exposure;
        out.push_back(day);
    }
    return out;
}

}  // namespace quant
