// quant_trend: one-shot, ledgered evaluation of the frozen five-sleeve trend rule.
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>

#include "quant/ledger.hpp"
#include "quant/trend.hpp"

using namespace quant;

int main(int argc, char** argv) {
    try {
        if (argc != 4) { std::cerr << "usage: quant_trend <run_dir> <contract> <data>\n"; return 2; }
        const std::string dir = argv[1];
        Json k = parse_json(read_file(argv[2]));
        TrendParams p;
        p.from = date_to_int(k.at("from").str());
        p.to = date_to_int(k.at("to").str());
        p.sma_months = static_cast<int>(k.at("sma_months").num());
        p.drift_band = k.at("drift_band").num();
        p.notional = k.at("notional_usd").num();
        p.commission = k.at("commission_usd").num();
        p.half_spread_bps = k.at("half_spread_bps").num();
        const double mult = k.at("elevated_mult").num();
        const uint64_t seed = static_cast<uint64_t>(k.at("seed").num());
        const int draws = static_cast<int>(k.at("bootstrap_draws").num());
        std::vector<std::string> syms;
        for (auto& s : k.at("symbols").a) syms.push_back(s.str());

        std::filesystem::create_directories(dir);
        Ledger l(dir + "/ledger.tsv");
        if (!l.last_of("freeze"))
            l.append("freeze", "{\"contract\":" + k.dump() + ",\"data_sha256\":" + Json::quote(sha256(read_file(argv[3]))) +
                                   ",\"engine_sha256\":" + Json::quote(sha256(read_file("include/quant/trend.hpp") + read_file("src/trend_run.cpp"))) + "}");
        if (!claim_receipt(dir + "/evaluation.receipt", k.at("hypothesis").str() + "\n")) throw std::runtime_error("this test has already been run; a changed rule needs a new run");
        l.append("evaluation_receipt", "{}");

        struct Row { int d; size_t i; double o, a; };
        std::vector<Row> rows;
        std::set<int> ds;
        std::ifstream f(argv[3]);
        std::string line;
        std::getline(f, line);
        while (std::getline(f, line)) {
            std::stringstream ss(line);
            std::string date, sym, o, a;
            std::getline(ss, date, ','); std::getline(ss, sym, ','); std::getline(ss, o, ','); std::getline(ss, a, ',');
            auto it = std::find(syms.begin(), syms.end(), sym);
            if (it == syms.end()) continue;
            int d = date_to_int(date);
            ds.insert(d);
            rows.push_back({d, static_cast<size_t>(it - syms.begin()), std::stod(o), std::stod(a)});
        }
        TrendData d;
        d.dates.assign(ds.begin(), ds.end());
        d.T = d.dates.size(); d.N = syms.size();
        d.open.assign(d.T * d.N, kNaN); d.adj.assign(d.T * d.N, kNaN);
        for (auto& r : rows) {
            size_t t = static_cast<size_t>(std::lower_bound(d.dates.begin(), d.dates.end(), r.d) - d.dates.begin());
            d.open[d.ix(t, r.i)] = r.o; d.adj[d.ix(t, r.i)] = r.a;
        }
        auto a = simulate_trend(d, p, false), o = simulate_trend(d, p, true);
        Evaluation strict = evaluate(a, o, {static_cast<int>(k.at("min_trades").num()), static_cast<int>(k.at("min_weeks").num())}, k.at("strict_alpha").num(), mult, seed);
        AdaptedResult ad = evaluate_active(a, o, k.at("adapted_alpha").num(), mult, seed, draws, true);
        std::map<long, double> monthly;
        double peak = 1, nav = 1, dd = 0, bpeak = 1, bnav = 1, bdd = 0, expo = 0;
        for (auto& x : a) {
            monthly[month_key(x.date)] += x.gross - x.cost - x.bench;
            nav *= 1 + x.gross - x.cost; peak = std::max(peak, nav); dd = std::min(dd, nav / peak - 1);
            bnav *= 1 + x.bench; bpeak = std::max(bpeak, bnav); bdd = std::min(bdd, bnav / bpeak - 1);
            expo += x.exposure;
        }
        std::vector<double> mv; for (auto& [m, x] : monthly) mv.push_back(x);
        const double dsr = deflated_sharpe(mv, static_cast<int>(k.at("deflated_trials").num()));
        l.append("result", "{\"strict\":" + strict.json() + ",\"adapted_passed\":" + (ad.passed() ? "true" : "false") + ",\"deflated_sharpe\":" + std::to_string(dsr) + "}");
        std::printf("first=%d last=%d avg_exposure=%.1f%% max_drawdown=%.1f%% benchmark_max_drawdown=%.1f%% cagr_like_total=%+.1f%% bench_total=%+.1f%%\n", a.front().date, a.back().date, expo / a.size() * 100, dd * 100, bdd * 100, (nav - 1) * 100, (bnav - 1) * 100);
        std::printf("=== STRICT BAR (%s) ===\n", strict.passed() ? "PASS" : "FAIL");
        std::printf("days=%d orders=%d net=%+.3fbp/d excess=%+.3fbp/d excess_lb=%+.2fbp/wk alpha_lb=%+.2fbp/wk elevated=%+.3fbp/d worst_year=%+.3fbp/d opposite=%+.3fbp/d net_sharpe=%+.2f beta=%.2f\n",
                    strict.days, strict.trades, strict.net_mean * 1e4, strict.excess_mean * 1e4, strict.excess_lb * 1e4, strict.alpha_lb * 1e4, strict.elevated_excess * 1e4, strict.worst_year * 1e4, strict.opposite_excess * 1e4, strict.sharpe, strict.beta);
        for (auto& [g, ok] : strict.gates) std::printf("  %-42s %s\n", g.c_str(), ok ? "pass" : "FAIL");
        std::printf("=== ADAPTED BAR (%s) ===\n", ad.passed() ? "PASS" : "FAIL");
        std::printf("net=%+.3fbp/d doubled_cost=%+.3fbp/d active=%+.3fbp/d weekly_lb=%+.2fbp monthly_lb=%+.2fbp opp_active=%+.3fbp/d long_minus_opp_lb=%+.2fbp worst_year_abs=%+.2f%% deflated_sharpe=%.3f\n",
                    ad.net_daily * 1e4, ad.elevated_daily * 1e4, ad.active_daily * 1e4, ad.weekly_lb * 1e4, ad.monthly_lb * 1e4, ad.opp_active_daily * 1e4, ad.long_minus_opp_weekly_lb * 1e4, ad.worst_year * 100, dsr);
        for (auto& [g, ok] : ad.gates) std::printf("  %-42s %s\n", g.c_str(), ok ? "pass" : "FAIL");
        std::map<int, std::pair<double, double>> yr;
        for (auto& x : a) { yr[x.date / 10000].first += x.gross - x.cost; yr[x.date / 10000].second += x.bench; }
        std::printf("calendar-year simple sums, net vs benchmark:");
        for (auto& [y, v] : yr) std::printf(" %d:%+.1f/%+.1f", y, v.first * 100, v.second * 100);
        std::printf("\nexecution_authorized=false\n");
        return 0;
    } catch (const std::exception& e) { std::cerr << "refused: " << e.what() << "\n"; return 1; }
}
