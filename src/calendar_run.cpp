// quant_calendar: one-shot, ledgered evaluation of the frozen turn-of-month rule.
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "quant/calendar.hpp"
#include "quant/ledger.hpp"

using namespace quant;

namespace {
void print_period(const char* name, const std::vector<int>& dates, const std::vector<double>& adj, const CalendarCost& c, int from, int to) {
    auto d = calendar_days(dates, adj, from, to, c, false);
    double net = 0, bench = 0, w = 0, o = 0; int nw = 0, no = 0, orders = 0;
    for (auto& x : d) { net += x.gross - x.cost; bench += x.bench; orders += x.tickets; if (x.exposure > 0) { w += x.bench; ++nw; } else { o += x.bench; ++no; } }
    std::printf("  %-22s net_sum=%+7.1f%%  buy_hold_sum=%+7.1f%%  window=%+6.2fbp/d  other=%+6.2fbp/d  orders=%d\n", name, net * 100, bench * 100, nw ? w / nw * 1e4 : 0, no ? o / no * 1e4 : 0, orders);
}
}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 4) { std::cerr << "usage: quant_calendar <run_dir> <contract> <data>\n"; return 2; }
        const std::string dir = argv[1];
        Json k = parse_json(read_file(argv[2]));
        const int from = date_to_int(k.at("from").str()), to = date_to_int(k.at("to").str());
        CalendarCost c{k.at("notional_usd").num(), k.at("commission_usd").num(), k.at("half_spread_bps").num()};
        const double mult = k.at("elevated_mult").num(), exposure = k.at("benchmark_exposure").num();
        const uint64_t seed = static_cast<uint64_t>(k.at("seed").num());
        const int draws = static_cast<int>(k.at("bootstrap_draws").num());
        std::filesystem::create_directories(dir);
        Ledger l(dir + "/ledger.tsv");
        if (!l.last_of("freeze"))
            l.append("freeze", "{\"contract\":" + k.dump() + ",\"data_sha256\":" + Json::quote(sha256(read_file(argv[3]))) +
                                   ",\"engine_sha256\":" + Json::quote(sha256(read_file("include/quant/calendar.hpp") + read_file("src/calendar_run.cpp"))) + "}");
        if (!claim_receipt(dir + "/evaluation.receipt", k.at("hypothesis").str() + "\n")) throw std::runtime_error("this test has already been run; a changed rule needs a new run");
        l.append("evaluation_receipt", "{}");
        std::vector<int> dates; std::vector<double> adj;
        std::ifstream f(argv[3]); std::string line; std::getline(f, line);
        while (std::getline(f, line)) { std::stringstream ss(line); std::string a, b, e; std::getline(ss, a, ','); std::getline(ss, b, ','); std::getline(ss, e, ','); dates.push_back(date_to_int(a)); adj.push_back(std::stod(e)); }
        auto lng = calendar_days(dates, adj, from, to, c, false);
        auto opp = calendar_days(dates, adj, from, to, c, true);
        Evaluation strict = evaluate(lng, opp, {static_cast<int>(k.at("min_trades").num()), static_cast<int>(k.at("min_weeks").num())}, k.at("strict_alpha").num(), mult, seed);
        AdaptedResult ad = evaluate_adapted(lng, opp, exposure, k.at("adapted_alpha").num(), mult, seed, draws);
        std::map<long, double> monthly; int invested = 0;
        for (auto& d : lng) { monthly[month_key(d.date)] += d.gross - d.cost; invested += d.exposure > 0; }
        std::vector<double> mv; for (auto& [m, v] : monthly) mv.push_back(v);
        const double dsr = deflated_sharpe(mv, static_cast<int>(k.at("deflated_trials").num()));
        l.append("result", "{\"strict\":" + strict.json() + ",\"adapted_passed\":" + (ad.passed() ? "true" : "false") + ",\"deflated_sharpe\":" + std::to_string(dsr) + "}");
        std::printf("=== STRICT BAR (%s) ===\n", strict.passed() ? "PASS" : "FAIL");
        std::printf("days=%d orders=%d invested=%.1f%% net=%+.3fbp/d excess_vs_buy_hold=%+.3fbp/d excess_lb=%+.2fbp/wk alpha_lb=%+.2fbp/wk elevated=%+.3fbp/d worst_year=%+.3fbp/d opposite=%+.3fbp/d net_sharpe=%+.2f beta=%.2f\n",
                    strict.days, strict.trades, 100.0 * invested / lng.size(), strict.net_mean * 1e4, strict.excess_mean * 1e4, strict.excess_lb * 1e4, strict.alpha_lb * 1e4, strict.elevated_excess * 1e4, strict.worst_year * 1e4, strict.opposite_excess * 1e4, strict.sharpe, strict.beta);
        for (auto& [g, ok] : strict.gates) std::printf("  %-42s %s\n", g.c_str(), ok ? "pass" : "FAIL");
        std::printf("=== ADAPTED BAR (%s) ===\n", ad.passed() ? "PASS" : "FAIL");
        std::printf("net=%+.3fbp/d doubled_cost=%+.3fbp/d active_vs_0.19xSPY=%+.3fbp/d weekly_lb=%+.2fbp monthly_lb=%+.2fbp opp_active=%+.3fbp/d long_minus_opp_lb=%+.2fbp worst_year_net=%+.2f%%\n",
                    ad.net_daily * 1e4, ad.elevated_daily * 1e4, ad.active_daily * 1e4, ad.weekly_lb * 1e4, ad.monthly_lb * 1e4, ad.opp_active_daily * 1e4, ad.long_minus_opp_weekly_lb * 1e4, ad.worst_year * 100);
        std::printf("window_mean=%+.2fbp/d other_days=%+.2fbp/d difference_t=%+.2f (report threshold about 2.58) deflated_sharpe=%.3f\n", ad.window_mean * 1e4, ad.other_mean * 1e4, ad.difference_t, dsr);
        for (auto& [g, ok] : ad.gates) std::printf("  %-42s %s\n", g.c_str(), ok ? "pass" : "FAIL");
        std::printf("=== REPORT-ONLY SUBPERIODS (do not affect the verdict) ===\n");
        print_period("1993-2005 (in-sample)", dates, adj, c, 19930201, 20051230);
        print_period("2006-2015", dates, adj, c, 20060103, 20151231);
        print_period("2016-2025", dates, adj, c, 20160104, 20251231);
        print_period("2026-07 onward", dates, adj, c, 20260701, 20261106);
        std::printf("execution_authorized=false\n");
        return 0;
    } catch (const std::exception& e) { std::cerr << "refused: " << e.what() << "\n"; return 1; }
}
