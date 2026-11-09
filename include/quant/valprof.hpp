#pragma once
// Long-only value-plus-profitability rule built from first-reported 10-K facts.
// A fact is usable at a formation date only if it was filed strictly before it.
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "calendar.hpp"

namespace quant {

struct Fact { int end, filed; double val; };

struct Company {
    std::map<std::string, std::vector<Fact>> tags;

    const Fact* at_end(const std::string& tag, int end, int before) const {
        auto it = tags.find(tag);
        if (it == tags.end()) return nullptr;
        const Fact* best = nullptr;
        for (auto& f : it->second)
            if (f.end == end && f.filed < before && (!best || f.filed < best->filed)) best = &f;
        return best;
    }

    // Latest fiscal year-end falling in `year` that was already public.
    int fiscal_end(int year, int before) const {
        auto it = tags.find("Assets");
        if (it == tags.end()) return 0;
        int best = 0;
        for (auto& f : it->second) if (f.end / 10000 == year && f.filed < before) best = std::max(best, f.end);
        return best;
    }

    double first_of(const std::vector<std::string>& names, int end, int before) const {
        for (auto& n : names) if (auto* f = at_end(n, end, before)) return f->val;
        return kNaN;
    }

    double gross_profit(int end, int before) const {
        if (auto* f = at_end("GrossProfit", end, before)) return f->val;
        const double rev = first_of({"Revenues", "RevenueFromContractWithCustomerExcludingAssessedTax", "SalesRevenueNet"}, end, before);
        const double cost = first_of({"CostOfRevenue", "CostOfGoodsAndServicesSold", "CostOfGoodsSold"}, end, before);
        return rev - cost;
    }

    // Most recently filed share count that was public on `date`.
    double shares(int date) const {
        auto it = tags.find("Shares");
        if (it == tags.end()) return kNaN;
        const Fact* best = nullptr;
        for (auto& f : it->second)
            if (f.filed <= date && (!best || f.filed > best->filed || (f.filed == best->filed && f.end > best->end))) best = &f;
        return best ? best->val : kNaN;
    }
};

struct Candidate { size_t index; double market_cap, bm, gpa, score = 0; };

// Ranks ascending on each metric and sums. Higher score is better.
inline void score_candidates(std::vector<Candidate>& c) {
    std::vector<size_t> order(c.size());
    auto apply = [&](double Candidate::*m) {
        for (size_t i = 0; i < c.size(); ++i) order[i] = i;
        std::sort(order.begin(), order.end(), [&](size_t a, size_t b) { return c[a].*m != c[b].*m ? c[a].*m < c[b].*m : c[a].index < c[b].index; });
        for (size_t r = 0; r < order.size(); ++r) c[order[r]].score += static_cast<double>(r);
    };
    for (auto& x : c) x.score = 0;
    apply(&Candidate::bm);
    apply(&Candidate::gpa);
}

// Applies the buy/hold band and returns the names to hold after this formation.
inline std::vector<size_t> apply_band(std::vector<Candidate> ranked, const std::set<size_t>& held, size_t positions, bool invert) {
    std::sort(ranked.begin(), ranked.end(), [&](const Candidate& a, const Candidate& b) {
        if (a.score != b.score) return invert ? a.score < b.score : a.score > b.score;
        return a.index < b.index;
    });
    const size_t n = ranked.size();
    const size_t keep_n = static_cast<size_t>(std::ceil(0.20 * static_cast<double>(n)));
    const size_t buy_n = static_cast<size_t>(std::ceil(0.10 * static_cast<double>(n)));
    std::vector<size_t> out;
    for (size_t i = 0; i < std::min(keep_n, n); ++i) if (held.count(ranked[i].index)) out.push_back(ranked[i].index);
    for (size_t i = 0; i < std::min(buy_n, n) && out.size() < positions; ++i)
        if (std::find(out.begin(), out.end(), ranked[i].index) == out.end() && !held.count(ranked[i].index)) out.push_back(ranked[i].index);
    return out;
}

struct PriceTable {
    std::vector<int> dates;
    std::vector<std::string> symbols;
    size_t T = 0, N = 0;
    std::vector<double> open, adj, raw, volume;
    size_t ix(size_t t, size_t i) const { return t * N + i; }
};

struct ValProfParams {
    size_t universe = 1000, positions = 10;
    double min_price = 5, min_dollar_volume = 20e6, notional = 2460, commission = 0.35, half_spread_bps = 3;
    int first_year = 2012, to = 20261106;
};

// Eligible, scored candidates at formation row F, using nothing after that row.
inline std::vector<Candidate> form_candidates(const PriceTable& p, const std::vector<Company>& co, size_t F, size_t dec, int year, const ValProfParams& v) {
    const int fdate = p.dates[F], decdate = p.dates[dec];
    std::vector<Candidate> c;
    std::vector<double> dv;
    for (size_t i = 0; i < p.N; ++i) {
        const double px = p.raw[p.ix(F, i)], px_dec = p.raw[p.ix(dec, i)];
        if (!(px >= v.min_price) || !(px_dec > 0) || F < 60) continue;
        dv.clear();
        for (size_t t = F + 1 - 60; t <= F; ++t) {
            const double a = p.raw[p.ix(t, i)], b = p.volume[p.ix(t, i)];
            if (a > 0 && b >= 0) dv.push_back(a * b);
        }
        if (dv.size() < 40) continue;
        std::nth_element(dv.begin(), dv.begin() + static_cast<long>(dv.size() / 2), dv.end());
        if (!(dv[dv.size() / 2] >= v.min_dollar_volume)) continue;
        const int end = co[i].fiscal_end(year - 1, fdate);
        if (!end) continue;
        const double sh_now = co[i].shares(fdate), sh_dec = co[i].shares(decdate);
        const auto* assets = co[i].at_end("Assets", end, fdate);
        const auto* equity = co[i].at_end("StockholdersEquity", end, fdate);
        const double gp = co[i].gross_profit(end, fdate);
        if (!assets || !equity || !(assets->val > 0) || !(equity->val > 0) || !std::isfinite(gp) || !(sh_now > 0) || !(sh_dec > 0)) continue;
        c.push_back({i, sh_now * px, equity->val / (sh_dec * px_dec), gp / assets->val});
    }
    std::sort(c.begin(), c.end(), [](const Candidate& a, const Candidate& b) { return a.market_cap != b.market_cap ? a.market_cap > b.market_cap : a.index < b.index; });
    if (c.size() > v.universe) c.resize(v.universe);
    score_candidates(c);
    return c;
}

struct Formation { int date; size_t universe, held, orders; };

inline std::vector<Day> simulate_valprof(const PriceTable& p, const std::vector<Company>& co, const ValProfParams& v, bool invert, std::vector<Formation>* log = nullptr) {
    std::vector<std::pair<size_t, size_t>> forms;  // Formation row and prior-December row.
    for (size_t t = 0; t + 1 < p.T; ++t) {
        const int ym = p.dates[t] / 100;
        if (p.dates[t] / 10000 < v.first_year || ym % 100 != 6 || p.dates[t + 1] / 100 == ym) continue;
        size_t dec = t;
        while (dec > 0 && p.dates[dec] / 100 != (p.dates[t] / 10000 - 1) * 100 + 12) --dec;
        if (p.dates[dec] / 100 == (p.dates[t] / 10000 - 1) * 100 + 12) forms.emplace_back(t, dec);
    }
    std::vector<Day> out;
    if (forms.empty()) return out;
    std::vector<double> w(p.N, 0.0), bw(p.N, 0.0);
    std::set<size_t> held;
    size_t nf = 0;
    const double order_cost = v.commission / v.notional, spread = v.half_spread_bps * 1e-4;
    for (size_t t = forms[0].first + 1; t + 1 < p.T && p.dates[t] <= v.to; ++t) {
        Day d;
        d.date = p.dates[t];
        if (nf < forms.size() && t == forms[nf].first + 1) {
            const size_t F = forms[nf].first;
            auto c = form_candidates(p, co, F, forms[nf].second, p.dates[F] / 10000, v);
            auto target = apply_band(c, held, v.positions, invert);
            std::set<size_t> next(target.begin(), target.end());
            double cash = 1.0;
            for (double x : w) cash -= x;
            size_t orders = 0;
            for (size_t i : held) if (!next.count(i)) {
                if (p.open[p.ix(t, i)] > 0) { cash += w[i]; d.cost += order_cost + w[i] * spread; w[i] = 0; ++orders; ++d.tickets; }
                else next.insert(i);  // Unpriced names cannot be sold today.
            }
            std::vector<size_t> buys;
            for (size_t i : next) if (!held.count(i) && p.open[p.ix(t, i)] > 0) buys.push_back(i);
            for (size_t i : buys) { w[i] = cash / static_cast<double>(buys.size()); d.cost += order_cost + w[i] * spread; ++orders; ++d.tickets; }
            held = next;
            std::fill(bw.begin(), bw.end(), 0.0);
            for (auto& x : c) bw[x.index] = 1.0 / static_cast<double>(c.size());
            if (log) log->push_back({p.dates[F], c.size(), held.size(), orders});
            ++nf;
        }
        double gross = 0, bench = 0, exposure = 0;
        std::vector<double> r(p.N, 0.0);
        for (size_t i = 0; i < p.N; ++i) {
            const double a = p.open[p.ix(t, i)], b = p.open[p.ix(t + 1, i)];
            if (a > 0 && b > 0) r[i] = b / a - 1;
            gross += w[i] * r[i]; bench += bw[i] * r[i]; exposure += w[i];
        }
        for (size_t i = 0; i < p.N; ++i) {
            w[i] = w[i] * (1 + r[i]) / (1 + gross);
            bw[i] = bw[i] * (1 + r[i]) / (1 + bench);
        }
        d.gross = gross; d.bench = bench; d.exposure = exposure;
        out.push_back(d);
    }
    return out;
}

// Adapted bar for fully invested rules: active return against the equal-weight universe.
inline AdaptedResult evaluate_active(const std::vector<Day>& d, const std::vector<Day>& opp, double alpha, double mult, uint64_t seed, int draws, bool absolute_years = false) {
    AdaptedResult r;
    std::vector<std::pair<int, double>> active, opp_active, diff;
    std::map<int, double> years;
    double net = 0, elev = 0, oa = 0;
    for (size_t i = 0; i < d.size(); ++i) {
        const double n = d[i].gross - d[i].cost;
        net += n; elev += d[i].gross - d[i].cost * mult;
        active.emplace_back(d[i].date, n - d[i].bench);
        years[d[i].date / 10000] += absolute_years ? n : n - d[i].bench;
    }
    for (size_t i = 0; i < d.size() && i < opp.size(); ++i) {
        const double n = opp[i].gross - opp[i].cost, a = n - opp[i].bench;
        oa += a;
        opp_active.emplace_back(opp[i].date, a);
        diff.emplace_back(d[i].date, (d[i].gross - d[i].cost) - n);
    }
    const double T = static_cast<double>(d.size());
    r.net_daily = net / T; r.elevated_daily = elev / T;
    for (auto& a : active) r.active_daily += a.second / T;
    r.opp_active_daily = oa / T;
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
        {"beats_equal_weight_universe", r.active_daily > 0},
        {"weekly_bootstrap_active_lb_positive", r.weekly_lb > 0},
        {"monthly_bootstrap_active_lb_positive", r.monthly_lb > 0},
        {absolute_years ? "no_negative_absolute_year" : "no_negative_active_year", no_neg},
        {"opposite_fails_active_test", !(r.opp_active_daily > 0 && opp_lb > 0)},
        {"long_minus_opposite_lb_positive", r.long_minus_opp_weekly_lb > 0},
    };
    return r;
}

}  // namespace quant
