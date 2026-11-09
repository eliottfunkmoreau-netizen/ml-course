// Offline tests: each one plants a failure the loop must catch, or a truth it must find.
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>

#include "quant/calendar.hpp"
#include "quant/gates.hpp"
#include "quant/ib_wire.hpp"
#include "quant/ledger.hpp"
#include "quant/otc_book.hpp"
#include "quant/trend.hpp"
#include "quant/valprof.hpp"

using namespace quant;
namespace fs_ = std::filesystem;

static int failures = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); ++failures; } } while (0)

static bool rejects(const std::string& t) { try { parse_hypothesis(t); return false; } catch (...) { return true; } }

static std::string prop(const std::string& id, const std::string& fam, const std::string& feature, const std::string& weight,
                        const std::string& top = "10", const std::string& extra = "") {
    return "{\"hypothesis\":{\"schema\":1,\"id\":\"" + id + "\",\"family\":\"" + fam + "\",\"model\":\"linear\",\"terms\":[{\"feature\":\"" + feature +
           "\",\"weight\":" + weight + "}],\"top_n\":" + top + ",\"rebalance_days\":5,\"market_filter\":\"none\"" + extra + "},\"rationale\":\"t\"}";
}

// plant=0: pure noise. plant>0: a stock's 5-day loser status predicts its next 5 days.
static Panel synthetic(size_t T, size_t N, double plant, unsigned seed) {
    Panel p;
    p.dates.resize(T);
    long day = days_from_civil(20190102);
    for (size_t t = 0; t < T; ++t) {
        while ((day + 4) % 7 == 5 || (day + 4) % 7 == 6) ++day;  // Skip weekends.
        long z = day + 719468, era = z / 146097; unsigned doe = z - era * 146097;
        unsigned yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365; int y = yoe + era * 400;
        unsigned doy = doe - (365*yoe + yoe/4 - yoe/100), mp = (5*doy + 2)/153, d = doy - (153*mp+2)/5 + 1, m = mp < 10 ? mp+3 : mp-9;
        p.dates[t] = (y + (m <= 2)) * 10000 + m * 100 + d;
        ++day;
    }
    for (size_t i = 0; i < N; ++i) p.symbols.push_back("S" + std::to_string(i));
    p.allocate(T, N);
    std::mt19937 rng(seed);
    std::normal_distribution<double> g(0, 0.012);
    for (size_t i = 0; i < N; ++i) {
        double px = 100;
        for (size_t t = 0; t < T; ++t) {
            double r = g(rng);
            if (plant > 0 && t >= 6) {
                double past = p.close[p.ix(t-1, i)] / p.close[p.ix(t-6, i)] - 1;
                r += -plant * past;
            }
            double open = px * (1 + g(rng) * 0.05);
            px = open * (1 + r);
            size_t k = p.ix(t, i);
            p.open[k] = open; p.close[k] = px; p.high[k] = std::max(open, px) * 1.002; p.low[k] = std::min(open, px) * 0.998;
            p.volume[k] = 1e6; p.ftotal[k] = 4e5; p.fshort[k] = 2e5;
        }
    }
    for (size_t t = 0; t < T; ++t) p.spy_close[t] = 100 + t * 0.01;
    return p;
}

int main() {
    CHECK(sha256("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    CHECK(sha256("") == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");

    // Boundary: only the fixed vocabulary is admissible, and there is no code field.
    CHECK(!rejects(prop("a", "f", "ret_5", "-1")));
    CHECK(rejects(prop("a", "f", "ret_5", "-1", "10", ",\"python\":\"import os\"")));
    CHECK(rejects(prop("a", "f", "future_return", "-1")));
    CHECK(rejects(prop("a", "f", "ret_5", "-0.73")));
    CHECK(rejects(prop("a", "f", "ret_5", "-1", "7")));
    CHECK(rejects("{\"hypothesis\":{},\"rationale\":\"x\",\"rationale\":\"y\"}"));

    // Admission: a turned knob is the same idea and must not spend a trial.
    {
        Admission a(3, 2);
        auto h1 = parse_hypothesis(prop("a", "f", "ret_5", "-1"));
        auto h2 = parse_hypothesis(prop("b", "f", "ret_5", "-0.5", "20"));
        auto h3 = parse_hypothesis(prop("c", "f", "ret_20", "-1"));
        auto h4 = parse_hypothesis(prop("d", "f", "ret_60", "-1"));
        auto h5 = parse_hypothesis(prop("e", "g", "ret_60", "1"));
        auto h6 = parse_hypothesis(prop("f", "h", "vol_20", "1"));
        CHECK(a.consider(h1).empty());
        CHECK(a.consider(h2) == "near_duplicate");
        CHECK(a.consider(h3).empty());
        CHECK(a.consider(h4) == "family_quota");
        CHECK(a.consider(h5).empty());
        CHECK(a.consider(h6) == "trial_budget_exhausted");
        CHECK(a.trials() == 3);
    }

    const std::string tmp = (fs_::temp_directory_path() / ("quant_test_" + std::to_string(::getpid()))).string();
    fs_::create_directories(tmp);

    // Ledger: any edit to history fails closed.
    {
        const std::string path = tmp + "/l.tsv";
        { Ledger l(path); l.append("a", "{\"x\":1}"); l.append("b", "{\"x\":2}"); l.append("c", "{\"x\":3}"); }
        { Ledger ok(path); CHECK(ok.events().size() == 3); }
        std::string text = read_file(path);
        std::string bad = text; bad.replace(bad.find("{\"x\":2}"), 7, "{\"x\":9}");
        std::ofstream(path, std::ios::trunc) << bad;
        bool threw = false;
        try { Ledger l(path); } catch (...) { threw = true; }
        CHECK(threw);
        std::string cut = text.substr(0, text.find('\n') + 1) + text.substr(text.find('\n', text.find('\n') + 1) + 1);
        std::ofstream(path, std::ios::trunc) << cut;
        threw = false;
        try { Ledger l(path); } catch (...) { threw = true; }
        CHECK(threw);
    }

    // Holdout: the capability can be claimed exactly once.
    CHECK(claim_receipt(tmp + "/r", "id\n"));
    CHECK(!claim_receipt(tmp + "/r", "id\n"));

    // Causality: rewriting the future must not change any feature or score up to t.
    {
        Panel p = synthetic(400, 40, 0, 11), q = p;
        for (size_t t = 300; t < 400; ++t) for (size_t i = 0; i < 40; ++i) {
            q.close[q.ix(t,i)] *= 3; q.open[q.ix(t,i)] *= 0.2; q.high[q.ix(t,i)] *= 5; q.volume[q.ix(t,i)] *= 9; q.ftotal[q.ix(t,i)] *= 2;
        }
        FeatureStore a = compute_features(p), b = compute_features(q);
        bool same = true;
        for (int f = 0; f < kFeatureCount; ++f) for (size_t t = 0; t < 300; ++t) for (size_t i = 0; i < 40; ++i) {
            double x = a.at(f,t,i), y = b.at(f,t,i);
            if (!(x == y || (std::isnan(x) && std::isnan(y)))) same = false;
        }
        CHECK(same);
        auto h = parse_hypothesis("{\"hypothesis\":{\"schema\":1,\"id\":\"r\",\"family\":\"m\",\"model\":\"ridge_wf\",\"terms\":[{\"feature\":\"ret_5\",\"weight\":1},{\"feature\":\"vol_20\",\"weight\":1}],\"top_n\":10,\"rebalance_days\":5,\"market_filter\":\"none\"},\"rationale\":\"t\"}");
        Scorer sa(p, a, h), sb(q, b, h);
        bool sc_same = true;
        for (size_t s : {200u, 250u, 299u}) {
            auto xa = sa.scores(s), xb = sb.scores(s);
            for (size_t i = 0; i < 40; ++i) if (!(xa[i] == xb[i] || (std::isnan(xa[i]) && std::isnan(xb[i])))) sc_same = false;
        }
        CHECK(sc_same);
    }

    // Stage truncation physically removes later rows.
    {
        Panel p = synthetic(100, 5, 0, 3);
        Panel c = p.up_to(p.dates[49]);
        CHECK(c.T == 50 && c.close.size() == 250 && c.dates.back() == p.dates[49]);
    }

    // Bootstrap is deterministic.
    {
        std::vector<std::pair<int,double>> d;
        std::mt19937 rng(5); std::normal_distribution<double> g(0, 0.01);
        Panel p = synthetic(300, 2, 0, 1);
        for (int date : p.dates) d.emplace_back(date, g(rng));
        CHECK(weekly_bootstrap_lb(d, 0.05, 42) == weekly_bootstrap_lb(d, 0.05, 42));
    }

    // Noise must almost never be accepted, across many independent tapes.
    {
        CostModel c{1'000'000, 0.0};  // Near-zero cost so rejection cannot be explained by friction.
        int accepted = 0, tapes = 40;
        auto h = parse_hypothesis(prop("n", "f", "ret_5", "-1", "10"));
        for (int s = 0; s < tapes; ++s) {
            Panel p = synthetic(1000, 60, 0, 100 + s);
            FeatureStore fs = compute_features(p);
            auto d = simulate(p, fs, h, c, p.dates[150], p.dates[998]);
            auto o = simulate(p, fs, h, c, p.dates[150], p.dates[998], true);
            if (evaluate(d, o, {120, 40}, 0.05, 2.0, 7).passed()) ++accepted;
        }
        std::printf("noise tapes accepted: %d/%d\n", accepted, tapes);
        CHECK(accepted <= 2);
    }

    // Positive control: a real planted reversal is found, and its opposite is refused.
    {
        CostModel c{1'000'000, 1.0};
        Panel p = synthetic(1000, 60, 0.25, 77);
        FeatureStore fs = compute_features(p);
        auto rev = parse_hypothesis(prop("rev", "f", "ret_5", "-1", "10"));
        auto mom = parse_hypothesis(prop("mom", "f", "ret_5", "1", "10"));
        auto run = [&](const Hypothesis& h) {
            return evaluate(simulate(p, fs, h, c, p.dates[150], p.dates[998]), simulate(p, fs, h, c, p.dates[150], p.dates[998], true), {120, 40}, 0.05, 2.0, 7);
        };
        Evaluation good = run(rev), bad = run(mom);
        std::printf("planted reversal excess %.1f bp/d passed=%d; opposite passed=%d\n", good.excess_mean * 1e4, good.passed(), bad.passed());
        CHECK(good.passed());
        CHECK(!bad.passed());
    }

    // Event filter: idle days carry no position, and scoring keeps only active days.
    {
        Panel p = synthetic(400, 40, 0, 21);
        for (size_t i = 0; i < 40; ++i) p.close[p.ix(200, i)] = p.close[p.ix(199, i)] * (0.95 - 0.0005 * static_cast<double>(i));  // Market-wide crash at row 200.
        FeatureStore fs = compute_features(p);
        CHECK(fs.breadth1[200] <= -0.02 && market_allows(fs, p, "crash_1d", 200) && !market_allows(fs, p, "crash_1d", 150));
        auto h = parse_hypothesis("{\"hypothesis\":{\"schema\":1,\"id\":\"c\",\"family\":\"f\",\"model\":\"linear\",\"terms\":[{\"feature\":\"ret_1\",\"weight\":-1}],\"top_n\":10,\"rebalance_days\":1,\"market_filter\":\"crash_1d\"},\"rationale\":\"t\"}");
        auto d = simulate(p, fs, h, CostModel{2500, 3}, p.dates[100], p.dates[398]);
        auto a = active_days(d);
        bool entered = false;
        for (auto& day : a) if (day.date == p.dates[201] && day.exposure > 0.9) entered = true;
        CHECK(entered);
        CHECK(a.size() < 40 && a.size() >= 2);
    }

    // Turn-of-month: window placement, costs, complement, power and false positives.
    {
        Panel cal = synthetic(600, 2, 0, 5);
        const auto& dates = cal.dates;
        auto flags = window_flags(dates, dates[0], dates.back());
        for (size_t t = 2; t + 4 < dates.size(); ++t)
            if (dates[t] / 100 != dates[t + 1] / 100 && dates[t - 1] / 100 == dates[t] / 100) {
                CHECK(flags[t] == 1 && flags[t + 1] == 1 && flags[t + 2] == 1 && flags[t + 3] == 1);
                CHECK(flags[t - 1] == 0);
                CHECK(flags[t + 4] == 0);
                break;
            }
        std::vector<double> flat(600, 100.0);
        CalendarCost cc{2460, 0.35, 1};
        CHECK(std::fabs(cc.per_order() - (0.35 / 2460 + 1e-4)) < 1e-12);
        auto d0 = calendar_days(dates, flat, dates[10], dates.back(), cc, false);
        auto o0 = calendar_days(dates, flat, dates[10], dates.back(), cc, true);
        int tickets = 0;
        double costs = 0;
        bool overlap = false;
        for (size_t i = 0; i < d0.size(); ++i) { tickets += d0[i].tickets; costs += d0[i].cost; overlap = overlap || (d0[i].exposure > 0 && o0[i].exposure > 0); }
        CHECK(tickets % 2 == 0 && tickets > 0);
        CHECK(std::fabs(costs - tickets * cc.per_order()) < 1e-12);
        CHECK(!overlap);
        // Pure noise must rarely pass; a planted window premium must pass; drift alone must not.
        auto make = [&](unsigned seed, double planted, double drift) {
            Panel p = synthetic(5200, 1, 0, seed);
            std::vector<double> adj(5200, 100.0);
            auto w = window_flags(p.dates, p.dates[0], p.dates.back());
            std::mt19937 rng(seed);
            std::normal_distribution<double> g(0, 0.010);
            for (size_t t = 1; t < 5200; ++t) adj[t] = adj[t - 1] * (1 + g(rng) + drift + (w[t] ? planted : 0.0));
            return std::make_pair(p.dates, adj);
        };
        auto score = [&](unsigned seed, double planted, double drift) {
            auto [ds, adj] = make(seed, planted, drift);
            auto a = calendar_days(ds, adj, ds[2], ds.back(), cc, false);
            auto b = calendar_days(ds, adj, ds[2], ds.back(), cc, true);
            return evaluate_adapted(a, b, 0.19, 0.005, 2.0, 7, 2000).passed();
        };
        int noise = 0, power = 0, drift_only = 0;
        for (unsigned s = 0; s < 30; ++s) { noise += score(1000 + s, 0, 0); drift_only += score(2000 + s, 0, 0.0004); power += score(3000 + s, 0.003, 0.0002); }
        std::printf("turn-of-month: noise passed %d/30, drift-only passed %d/30, planted passed %d/30\n", noise, drift_only, power);
        CHECK(noise <= 1);
        CHECK(drift_only <= 1);
        CHECK(power >= 15);
        std::vector<double> sample;
        std::mt19937 rng(9);
        std::normal_distribution<double> g(0, 0.02);
        for (int i = 0; i < 250; ++i) sample.push_back(g(rng));
        CHECK(deflated_sharpe(sample, 21) < 0.5);
        CHECK(std::fabs(inv_norm(0.975) - 1.95996) < 1e-3);
    }

    // Value-profitability: point-in-time facts, the band, and a planted signal.
    {
        Company a;
        a.tags["Assets"] = {{20181231, 20190301, 100}, {20181231, 20190601, 999}, {20191231, 20200715, 500}};
        a.tags["GrossProfit"] = {{20181231, 20190301, 30}};
        a.tags["StockholdersEquity"] = {{20181231, 20190301, 40}};
        a.tags["Shares"] = {{20181231, 20190301, 10}, {20190331, 20190510, 12}, {20190630, 20190810, 99}};
        CHECK(a.fiscal_end(2018, 20190628) == 20181231);
        CHECK(a.fiscal_end(2019, 20200628) == 0);  // Filed after formation, so invisible.
        CHECK(a.at_end("Assets", 20181231, 20190628)->val == 100);  // First-reported value wins over the later one.
        CHECK(a.at_end("Assets", 20181231, 20190301) == nullptr);  // Filed strictly before, not on the day.
        CHECK(a.shares(20190628) == 12);
        CHECK(std::isnan(Company{}.shares(20190628)));
        Company r;
        r.tags["Revenues"] = {{20181231, 20190301, 80}};
        r.tags["CostOfRevenue"] = {{20181231, 20190301, 50}};
        CHECK(r.gross_profit(20181231, 20190628) == 30);

        std::vector<Candidate> c;
        for (size_t i = 0; i < 100; ++i) c.push_back({i, 1, static_cast<double>(i), static_cast<double>(i)});
        score_candidates(c);
        CHECK(c[99].score > c[0].score);
        auto first = apply_band(c, {}, 10, false);
        CHECK(first.size() == 10 && first[0] == 99 && first[9] == 90);
        auto low = apply_band(c, {}, 10, true);
        CHECK(low.size() == 10 && low[0] == 0);
        // Rank 85 is inside the top 20 percent and stays; rank 50 is sold and replaced.
        auto next = apply_band(c, {85, 50}, 10, false);
        CHECK(std::find(next.begin(), next.end(), 85) != next.end());
        CHECK(std::find(next.begin(), next.end(), 50) == next.end());
        CHECK(next.size() == 10);

        // End to end: firms with the best fundamentals are given higher returns.
        const size_t T = 1560, N = 120;
        Panel base = synthetic(T, 1, 0, 3);
        auto build = [&](double planted, unsigned seed) {
            PriceTable p;
            p.dates = base.dates; p.T = T; p.N = N;
            for (size_t i = 0; i < N; ++i) p.symbols.push_back("S" + std::to_string(i));
            p.open.assign(T * N, 0); p.adj.assign(T * N, 0); p.raw.assign(T * N, 0); p.volume.assign(T * N, 0);
            std::mt19937 rng(seed);
            std::normal_distribution<double> g(0, 0.01);
            for (size_t i = 0; i < N; ++i) {
                double px = 50;
                const double edge = planted * (static_cast<double>(i) / static_cast<double>(N) - 0.5);
                for (size_t t = 0; t < T; ++t) {
                    px *= 1 + g(rng) + edge;
                    p.open[p.ix(t, i)] = px; p.adj[p.ix(t, i)] = px; p.raw[p.ix(t, i)] = px; p.volume[p.ix(t, i)] = 2e6;
                }
            }
            return p;
        };
        PriceTable pl = build(0.0040, 11);
        std::vector<Company> co(N);
        // Book equity tracks each December price, so book/market stays a stable ranking.
        auto dec_row = [&](int y) {
            size_t r = 0;
            for (size_t t = 0; t < T; ++t) if (pl.dates[t] / 100 == y * 100 + 12) r = t;
            return r;
        };
        for (size_t i = 0; i < N; ++i) {
            for (int y = 2019; y <= 2025; ++y) {
                const int end = y * 10000 + 1231, filed = (y + 1) * 10000 + 301;
                co[i].tags["Assets"].push_back({end, filed, 1000});
                co[i].tags["GrossProfit"].push_back({end, filed, 10.0 + static_cast<double>(i)});
                co[i].tags["StockholdersEquity"].push_back({end, filed, pl.raw[pl.ix(dec_row(y), i)] * 1e6 * (1.0 + static_cast<double>(i) / static_cast<double>(N))});
                co[i].tags["Shares"].push_back({end, filed, 1e6});
            }
            co[i].tags["Shares"].push_back({20181231, 20190201, 1e6});
        }
        ValProfParams v;
        v.universe = 120; v.positions = 10; v.first_year = 2019; v.to = 99999999; v.min_dollar_volume = 1e6; v.notional = 1e9; v.commission = 0;
        std::vector<Formation> log;
        auto d = simulate_valprof(pl, co, v, false, &log);
        auto o = simulate_valprof(pl, co, v, true);
        CHECK(!log.empty() && log[0].held == 10 && log[0].universe == 120);
        auto res = evaluate_active(d, o, 0.005, 2.0, 7, 2000);
        std::printf("value-profitability planted: active %.1f bp/d passed=%d, formations=%zu\n", res.active_daily * 1e4, res.passed(), log.size());
        CHECK(res.passed());
        // Rewriting prices after formation must not alter that formation's selection.
        PriceTable fut = pl;
        size_t F = 0;
        while (pl.dates[F] < log[1].date) ++F;
        for (size_t t = F + 1; t < T; ++t) for (size_t i = 0; i < N; ++i) { fut.raw[fut.ix(t, i)] *= 3; fut.volume[fut.ix(t, i)] *= 5; }
        size_t dec = F;
        while (pl.dates[dec] / 100 != (pl.dates[F] / 10000 - 1) * 100 + 12) --dec;
        auto c1 = form_candidates(pl, co, F, dec, pl.dates[F] / 10000, v), c2 = form_candidates(fut, co, F, dec, pl.dates[F] / 10000, v);
        bool same = c1.size() == c2.size();
        for (size_t i = 0; i < c1.size() && same; ++i) same = c1[i].index == c2[i].index && c1[i].score == c2[i].score;
        CHECK(same);
    }

    // ETF trend: month-ends, causality, drift band, costs, complement, and null behaviour.
    {
        Panel base = synthetic(1300, 1, 0, 8);
        auto make = [&](unsigned seed, double drift, double vol) {
            TrendData d;
            d.dates = base.dates; d.T = 1300; d.N = 5;
            d.open.assign(1300 * 5, 0); d.adj.assign(1300 * 5, 0);
            std::mt19937 rng(seed);
            std::normal_distribution<double> g(0, vol);
            for (size_t i = 0; i < 5; ++i) {
                double px = 100;
                for (size_t t = 0; t < 1300; ++t) { px *= 1 + g(rng) + drift; d.open[d.ix(t, i)] = px; d.adj[d.ix(t, i)] = px; }
            }
            return d;
        };
        TrendData d = make(4, 0, 0.01);
        auto ends = month_end_rows(d.dates);
        CHECK(!ends.empty() && d.dates[ends[0]] / 100 != d.dates[ends[0] + 1] / 100 && d.dates[ends[0] - 1] / 100 == d.dates[ends[0]] / 100);
        CHECK(trend_signal(d, ends, 8, 0, 10) == -1);
        CHECK(trend_signal(d, ends, 9, 0, 10) >= 0);
        // A rising series is above trend; a falling series is below.
        TrendData up = d, down = d;
        for (size_t t = 0; t < 1300; ++t) { up.adj[up.ix(t, 0)] = 100 + static_cast<double>(t); down.adj[down.ix(t, 0)] = 2000 - static_cast<double>(t); }
        CHECK(trend_signal(up, ends, 20, 0, 10) == 1);
        CHECK(trend_signal(down, ends, 20, 0, 10) == 0);
        // Changing anything after a month-end cannot alter that month-end's signal.
        TrendData fut = d;
        for (size_t t = ends[20] + 1; t < 1300; ++t) for (size_t i = 0; i < 5; ++i) fut.adj[fut.ix(t, i)] *= 4;
        bool same = true;
        for (size_t i = 0; i < 5; ++i) same = same && trend_signal(d, ends, 20, i, 10) == trend_signal(fut, ends, 20, i, 10);
        CHECK(same);
        TrendParams tp;
        tp.from = d.dates[ends[9] + 1];
        auto a = simulate_trend(d, tp, false), b = simulate_trend(d, tp, true);
        CHECK(!a.empty() && a.size() == b.size());
        bool feasible = true;
        int tickets = 0;
        for (auto& day : a) { feasible = feasible && day.exposure <= 1.0 + 1e-9 && day.exposure >= -1e-9; tickets += day.tickets; }
        CHECK(feasible && tickets > 0);
        // The first rebalance buys exactly the sleeves that are above trend.
        int above = 0;
        for (size_t i = 0; i < 5; ++i) above += trend_signal(d, ends, 9, i, 10) == 1;
        CHECK(std::fabs(a[0].exposure - above * 0.2) < 1e-9);
        // Random walks with drift must not clear the bar against their own equal-weight basket.
        int nulls = 0;
        for (unsigned s = 0; s < 30; ++s) {
            TrendData n = make(5000 + s, 0.0003, 0.01);
            TrendParams q;
            q.from = n.dates[month_end_rows(n.dates)[9] + 1];
            nulls += evaluate_active(simulate_trend(n, q, false), simulate_trend(n, q, true), 0.005, 2.0, 7, 2000, true).passed();
        }
        std::printf("trend: random walks passed %d/30\n", nulls);
        CHECK(nulls <= 1);
    }

    // Small accounts pay more: a fixed minimum commission must bite.
    {
        Panel p = synthetic(300, 40, 0, 9);
        FeatureStore fs = compute_features(p);
        auto h = parse_hypothesis(prop("c", "f", "ret_5", "-1", "20"));
        double small = 0, large = 0;
        for (auto& d : simulate(p, fs, h, CostModel{2500, 3}, p.dates[50], p.dates[298])) small += d.cost;
        for (auto& d : simulate(p, fs, h, CostModel{1000000, 3}, p.dates[50], p.dates[298])) large += d.cost;
        CHECK(small > large * 1.5);
    }

    // IBKR wire: framing survives arbitrary splits, and decoded fields land correctly.
    {
        using namespace quant::ib;
        CHECK(handshake() == std::string("API\0\0\0\0\x09v100..187", 17));
        auto body = [](const std::string& fr) { return fr.substr(4); };
        const std::string rq_body = body(req_stock_quotes(7, "AAPL"));
        auto rq = split_fields(rq_body);
        CHECK(rq.size() == 20 && rq[0] == "1" && rq[1] == "11" && rq[2] == "7" && rq[4] == "AAPL" && rq[5] == "STK" && rq[10] == "SMART" && rq[12] == "USD");
        CHECK(split_fields(body(start_api(17)))[0] == "71");
        const std::string hist = body(req_daily_history(3, "IBM", "1 Y"));
        auto hf = split_fields(hist);
        CHECK(hf.size() == 23 && hf[0] == "20" && hf[1] == "3" && hf[3] == "IBM" && hf[9] == "SMART" && hf[16] == "1 day" && hf[17] == "1 Y" && hf[18] == "1" && hf[19] == "TRADES" && hf[20] == "1");
        std::string stream = be32(0);
        std::string tick("1\0" "6\0" "7\0" "1\0" "189.25\0" "300\0" "0\0", 25);
        std::string err("4\0" "2\0" "7\0" "354\0" "not subscribed\0" "\0", 27);
        stream = be32(static_cast<uint32_t>(tick.size())) + tick + be32(static_cast<uint32_t>(err.size())) + err;
        Framer fr;
        std::string out;
        std::vector<Event> seen;
        for (char c : stream) {  // Worst case: one byte per read.
            fr.feed(&c, 1);
            while (fr.next(out)) seen.push_back(decode(split_fields(out)));
        }
        CHECK(seen.size() == 2);
        if (seen.size() == 2) {
            CHECK(seen[0].kind == Kind::Price && seen[0].id == 7 && seen[0].tick == 1 && seen[0].value == 189.25);
            CHECK(seen[1].kind == Kind::Error && seen[1].id == 7 && seen[1].code == 354 && seen[1].text == "not subscribed");
        }
        bool oversized = false;
        Framer bad;
        std::string huge = be32(0x7fffffff);
        bad.feed(huge.data(), huge.size());
        try { bad.next(out); } catch (...) { oversized = true; }
        CHECK(oversized);
    }

    // OTC analytics: planted quotes and prints with hand-computed answers.
    {
        using namespace quant::otc;
        const int64_t t0 = 1'800'000'000'000;
        auto near = [](double a, double b) { return std::abs(a - b) < 1e-9; };
        Book b;
        b.start(t0);
        b.on_price(1, 1.00, t0); b.on_size(0, 3000, t0);
        b.on_price(2, 1.10, t0); b.on_size(3, 1000, t0);
        auto m = b.metrics(t0, 500, 0);
        CHECK(near(m.mid, 1.05) && near(m.spread_pct, 0.10 / 1.05 * 100));
        CHECK(near(m.micro, (1.00 * 1000 + 1.10 * 3000) / 4000.0));  // Heavy bid pulls the microprice toward the ask.
        CHECK(near(m.imbalance, 0.5));
        const double shares = 500 / 1.10, comm = std::min(5.0, std::max(0.35, 0.0035 * shares));
        CHECK(near(m.cost_pct, (0.10 * shares + 2 * comm) / 500 * 100));
        CHECK(near(m.fill_pct, 100));

        // A stale print replayed at subscribe must not count as flow.
        b.on_rt_volume("1.05;900;" + std::to_string(t0 - 86'400'000) + ";50000;1.03;true", t0 + 10);
        CHECK(b.trades().empty() && near(b.metrics(t0 + 10, 500, 0).vwap, 1.03));
        // Quote rule: at the ask is a buy, at the bid a sell, inside by distance to the mid.
        b.on_rt_volume("1.10;100;" + std::to_string(t0 + 1000) + ";50100;1.03;true", t0 + 1000);
        b.on_rt_volume("1.00;300;" + std::to_string(t0 + 2000) + ";50400;1.03;true", t0 + 2000);
        b.on_rt_volume("1.08;50;" + std::to_string(t0 + 3000) + ";50450;1.03;false", t0 + 3000);
        // Exactly at the mid falls through to the tick test: 1.05 < 1.08 is a sell.
        b.on_rt_volume("1.05;50;" + std::to_string(t0 + 4000) + ";50500;1.03;false", t0 + 4000);
        m = b.metrics(t0 + 5000, 500, 0);
        CHECK(m.flow_1m.trades == 4 && near(m.flow_1m.buy, 150) && near(m.flow_1m.sell, 350));
        CHECK(near(m.volume, 50500));
        CHECK(near(b.metrics(t0 + 120000, 500, 0).flow_1m.total, 0) && near(b.metrics(t0 + 120000, 500, 0).flow_5m.total, 500));

        // A missing side is not a zero price, and is ungradable.
        Book o;
        o.start(t0);
        o.on_price(1, -1, t0); o.on_price(2, 0.50, t0); o.on_size(3, 100, t0);
        m = o.metrics(t0, 500, 0);
        CHECK(!std::isfinite(m.spread_pct) && m.grade == 'F');
        CHECK(std::find(m.flags.begin(), m.flags.end(), "ONE-SIDED") != m.flags.end());

        // Delayed ticks land in the same fields, and a delayed last-size stands in for RTVolume.
        Book d;
        d.start(t0);
        d.on_price(66, 2.00, t0); d.on_price(67, 2.02, t0); d.on_price(68, 2.02, t0); d.on_size(71, 200, t0);
        CHECK(d.trades().empty());  // The snapshot at subscribe is not a print.
        d.on_size(71, 300, t0 + 1000);
        CHECK(d.trades().size() == 1 && d.trades().back().side == 1 && d.trades().back().sz == 300);
        Book rt;
        rt.start(t0);
        rt.on_price(4, 2.0, t0); rt.on_size(5, 100, t0); rt.on_size(5, 100, t0 + 1000);
        CHECK(rt.trades().empty());  // Realtime prints come only from RTVolume.

        // Time-weighted spread: 1% for 30s, then 3% for 30s, and 60s one-sided.
        Book w;
        w.start(t0);
        w.on_price(1, 0.995, t0); w.on_price(2, 1.005, t0);
        w.on_price(1, 0.985, t0 + 30000); w.on_price(2, 1.015, t0 + 30000);
        w.on_price(1, -1, t0 + 60000);
        m = w.metrics(t0 + 120000, 500, 0);
        CHECK(std::abs(m.tw_spread_pct - 2.0) < 1e-9 && std::abs(m.two_sided_pct - 50) < 1e-9);

        // Daily statistics, plus a pace that needs the fraction of the session elapsed.
        std::vector<Daily> hist(30);
        for (size_t i = 0; i < hist.size(); ++i) hist[i] = {1.0, 1.1, 0.9, 1.0, i < 25 ? 1000.0 : 0.0};
        auto s = daily_stats(hist);
        CHECK(s.ok && near(s.adv20, 750) && near(s.dollar_adv20, 750) && s.zero_days60 == 5 && near(s.atr14_pct, 20) && near(s.rv20_ann_pct, 0) && near(s.high_1y, 1.1) && near(s.low_1y, 0.9));
        CHECK(!daily_stats(std::vector<Daily>(20)).ok);
        Book p;
        p.start(t0); p.set_daily(s); p.on_size(8, 1500, t0); p.on_price(4, 1.0, t0);
        CHECK(near(p.metrics(t0, 500, 0.5).pace, 4));
        CHECK(near(commission(1'000'000, 500), 5));   // 1% cap on sub-penny tickets.
        CHECK(near(commission(10, 500), 0.35));       // Minimum.
    }

    // RTVolume and halt messages decode.
    {
        using namespace quant::ib;
        std::string tick("46\0" "6\0" "3\0" "48\0" "1.05;100;1;2;3;true\0", 30);
        Event e = decode(split_fields(tick));
        CHECK(e.kind == Kind::String && e.id == 3 && e.tick == 48 && e.text == "1.05;100;1;2;3;true");
        std::string halt("45\0" "6\0" "3\0" "49\0" "1\0", 12);
        e = decode(split_fields(halt));
        CHECK(e.kind == Kind::Generic && e.id == 3 && e.tick == 49 && e.value == 1);
        const std::string wire = req_quotes_with_trades(4, "FNMA").substr(4);
        auto qf = split_fields(wire);
        CHECK(qf.size() == 20 && qf[0] == "1" && qf[4] == "FNMA" && qf[16] == "233");
    }

    fs_::remove_all(tmp);
    std::printf(failures ? "%d FAILED\n" : "all tests passed\n", failures);
    return failures ? 1 : 0;
}
