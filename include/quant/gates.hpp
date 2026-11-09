#pragma once
// Acceptance is a conjunction of absolute gates. The ranking score only orders
// candidates that already passed; it can never admit one.
#include <algorithm>
#include <cmath>
#include <map>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "backtest.hpp"
#include "json.hpp"

namespace quant {

struct Floors { int trades; int weeks; };

struct Evaluation {
    std::vector<std::pair<std::string, bool>> gates;
    double net_mean = 0, excess_mean = 0, excess_lb = 0, alpha_lb = 0, elevated_excess = 0;
    double opposite_excess = 0, worst_year = 0, sharpe = 0, excess_sharpe = 0, beta = 0;
    double total_net = 0, total_bench = 0, avg_exposure = 0;
    int trades = 0, weeks = 0, days = 0;

    bool passed() const {
        if (gates.empty()) return false;
        return std::all_of(gates.begin(), gates.end(), [](auto& g) { return g.second; });
    }
    double rank_score() const { return std::min({excess_mean, excess_lb, elevated_excess, worst_year}); }

    std::string json() const {
        Json o; o.kind = Json::Obj;
        auto num = [&](const char* k, double v) { Json j; j.kind = Json::Num; j.n = std::isfinite(v) ? v : 0; o.o[k] = j; };
        num("days", days); num("trades", trades); num("weeks", weeks);
        num("net_daily_bps", net_mean * 1e4); num("excess_daily_bps", excess_mean * 1e4);
        num("excess_weekly_lb_bps", excess_lb * 1e4); num("alpha_weekly_lb_bps", alpha_lb * 1e4);
        num("elevated_excess_daily_bps", elevated_excess * 1e4); num("opposite_excess_daily_bps", opposite_excess * 1e4);
        num("worst_year_excess_daily_bps", worst_year * 1e4); num("net_sharpe", sharpe);
        num("excess_sharpe", excess_sharpe); num("beta", beta); num("total_net_pct", total_net * 100);
        num("total_bench_pct", total_bench * 100); num("avg_exposure", avg_exposure);
        Json g; g.kind = Json::Obj;
        for (auto& [name, ok] : gates) { Json b; b.kind = Json::Bool; b.b = ok; g.o[name] = b; }
        o.o["gates"] = g;
        Json p; p.kind = Json::Bool; p.b = passed(); o.o["passed"] = p;
        return o.dump();
    }
};

inline long days_from_civil(int yyyymmdd) {
    int y = yyyymmdd / 10000; unsigned m = (yyyymmdd / 100) % 100, d = yyyymmdd % 100;
    y -= m <= 2;
    const long era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<long>(doe) - 719468;
}

// Monday-based week index: 1970-01-01 was a Thursday.
inline long week_key(int yyyymmdd) { return (days_from_civil(yyyymmdd) + 3) / 7; }

// Lower `alpha` quantile of the mean weekly P&L, resampling whole weeks.
inline double weekly_bootstrap_lb(const std::vector<std::pair<int, double>>& daily, double alpha, uint64_t seed, int resamples = 2000) {
    std::map<long, double> by_week;
    for (auto& [date, v] : daily) by_week[week_key(date)] += v;
    std::vector<double> weeks;
    for (auto& [k, v] : by_week) weeks.push_back(v);
    if (weeks.size() < 2) return -1;
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<size_t> pick(0, weeks.size() - 1);
    std::vector<double> means(static_cast<size_t>(resamples));
    for (auto& m : means) {
        double s = 0;
        for (size_t i = 0; i < weeks.size(); ++i) s += weeks[pick(rng)];
        m = s / static_cast<double>(weeks.size());
    }
    std::sort(means.begin(), means.end());
    size_t idx = std::min(means.size() - 1, static_cast<size_t>(alpha * static_cast<double>(means.size())));
    return means[idx];
}

// Event-conditional rules are scored on days they trade, so idle cash is not counted
// as underperformance. Every day with a position or a trading cost stays in the sample.
inline std::vector<Day> active_days(const std::vector<Day>& days) {
    std::vector<Day> out;
    for (auto& d : days) if (d.exposure > 0 || d.cost > 0) out.push_back(d);
    return out;
}

inline Evaluation evaluate(const std::vector<Day>& days, const std::vector<Day>& opposite, const Floors& floors,
                           double alpha, double elevated_mult, uint64_t seed, int min_year_days = 40) {
    Evaluation ev;
    ev.days = static_cast<int>(days.size());
    if (days.empty()) { ev.gates.emplace_back("sample_nonempty", false); return ev; }
    std::vector<std::pair<int, double>> excess, residual;
    std::map<int, std::pair<double, int>> years;
    std::map<long, int> weeks;
    double sn = 0, sn2 = 0, se = 0, se2 = 0, sb = 0, sb2 = 0, snb = 0, elev = 0, wn = 1, wb = 1, expo = 0;
    for (auto& d : days) {
        double net = d.gross - d.cost, ex = net - d.bench;
        sn += net; sn2 += net * net; se += ex; se2 += ex * ex;
        sb += d.bench; sb2 += d.bench * d.bench; snb += net * d.bench;
        elev += d.gross - d.cost * elevated_mult - d.bench;
        wn *= 1 + net; wb *= 1 + d.bench; expo += d.exposure;
        excess.emplace_back(d.date, ex);
        auto& y = years[d.date / 10000]; y.first += ex; y.second += 1;
        ev.trades += d.tickets;
        if (d.exposure > 0) weeks[week_key(d.date)] += 1;
    }
    const double n = static_cast<double>(days.size());
    ev.weeks = static_cast<int>(weeks.size());
    ev.net_mean = sn / n; ev.excess_mean = se / n; ev.elevated_excess = elev / n;
    ev.total_net = wn - 1; ev.total_bench = wb - 1; ev.avg_exposure = expo / n;
    double nvar = sn2 / n - ev.net_mean * ev.net_mean, evar = se2 / n - ev.excess_mean * ev.excess_mean;
    ev.sharpe = nvar > 0 ? ev.net_mean / std::sqrt(nvar) * std::sqrt(252.0) : 0;
    ev.excess_sharpe = evar > 0 ? ev.excess_mean / std::sqrt(evar) * std::sqrt(252.0) : 0;
    double bvar = sb2 / n - (sb / n) * (sb / n);
    ev.beta = bvar > 0 ? (snb / n - ev.net_mean * (sb / n)) / bvar : 0;
    // Residual keeps the intercept: this is the P&L that is not the market's own drift.
    for (auto& d : days) residual.emplace_back(d.date, d.gross - d.cost - ev.beta * d.bench);
    ev.excess_lb = weekly_bootstrap_lb(excess, alpha, seed);
    ev.alpha_lb = weekly_bootstrap_lb(residual, alpha, seed + 1);
    double opp = 0;
    for (auto& d : opposite) opp += d.gross - d.cost - d.bench;
    ev.opposite_excess = opposite.empty() ? 0 : opp / static_cast<double>(opposite.size());
    bool no_negative_year = true;
    ev.worst_year = 1e9;
    for (auto& [y, st] : years) {
        if (st.second < min_year_days) continue;
        double m = st.first / st.second;
        ev.worst_year = std::min(ev.worst_year, m);
        no_negative_year = no_negative_year && m > 0;
    }
    if (ev.worst_year > 1e8) ev.worst_year = ev.excess_mean;
    bool finite = std::isfinite(ev.net_mean) && std::isfinite(ev.excess_mean) && std::isfinite(ev.excess_lb) && std::isfinite(ev.alpha_lb);
    ev.gates = {
        {"finite_metrics", finite},
        {"net_pnl_positive", ev.net_mean > 0},
        {"excess_over_equal_weight_positive", ev.excess_mean > 0},
        {"weekly_bootstrap_excess_lb_positive", ev.excess_lb > 0},
        {"drift_neutral_alpha_lb_positive", ev.alpha_lb > 0},
        {"elevated_cost_excess_positive", ev.elevated_excess > 0},
        {"no_negative_year", no_negative_year},
        {"opposite_side_does_not_pass", ev.opposite_excess <= 0},
        {"trade_floor", ev.trades >= floors.trades},
        {"week_floor", ev.weeks >= floors.weeks},
    };
    return ev;
}

}  // namespace quant
