// quant_valprof: one-shot, ledgered evaluation of the frozen value-profitability rule.
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <unordered_map>

#include "quant/ledger.hpp"
#include "quant/valprof.hpp"

using namespace quant;

namespace {

std::vector<std::string> split(const std::string& l) {
    std::vector<std::string> o; std::string c; std::stringstream s(l);
    while (std::getline(s, c, ',')) o.push_back(c);
    // Python's csv writer ends rows with CRLF; the last column must not keep the CR.
    if (!o.empty() && !o.back().empty() && o.back().back() == '\r') o.back().pop_back();
    return o;
}

int iso(const std::string& d) { return date_to_int(d); }

}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 5) { std::cerr << "usage: quant_valprof <run_dir> <contract> <facts> <prices>\n"; return 2; }
        const std::string dir = argv[1];
        Json k = parse_json(read_file(argv[2]));
        ValProfParams v;
        v.universe = static_cast<size_t>(k.at("universe_size").num());
        v.positions = static_cast<size_t>(k.at("positions").num());
        v.min_price = k.at("min_price").num();
        v.min_dollar_volume = k.at("min_median_dollar_volume").num();
        v.notional = k.at("notional_usd").num();
        v.commission = k.at("commission_usd").num();
        v.half_spread_bps = k.at("half_spread_bps").num();
        v.first_year = static_cast<int>(k.at("first_year").num());
        v.to = date_to_int(k.at("to").str());
        const double mult = k.at("elevated_mult").num();
        const uint64_t seed = static_cast<uint64_t>(k.at("seed").num());
        const int draws = static_cast<int>(k.at("bootstrap_draws").num());

        std::filesystem::create_directories(dir);
        Ledger l(dir + "/ledger.tsv");
        if (!l.last_of("freeze"))
            l.append("freeze", "{\"contract\":" + k.dump() + ",\"facts_sha256\":" + Json::quote(sha256(read_file(argv[3]))) + ",\"prices_sha256\":" + Json::quote(sha256(read_file(argv[4]))) +
                                   ",\"engine_sha256\":" + Json::quote(sha256(read_file("include/quant/valprof.hpp") + read_file("src/valprof_run.cpp"))) + "}");
        if (!claim_receipt(dir + "/evaluation.receipt", k.at("hypothesis").str() + "\n")) throw std::runtime_error("this test has already been run; a changed rule needs a new run");
        l.append("evaluation_receipt", "{}");

        std::unordered_map<std::string, size_t> sym_ix;
        std::vector<Company> co;
        PriceTable p;
        std::string line;
        {
            std::ifstream f(argv[3]); std::getline(f, line);
            while (std::getline(f, line)) {
                auto c = split(line);
                if (c.size() != 7) continue;
                auto [it, fresh] = sym_ix.emplace(c[1], co.size());
                if (fresh) { co.emplace_back(); p.symbols.push_back(c[1]); }
                co[it->second].tags[c[3]].push_back({iso(c[4]), iso(c[6]), std::stod(c[5])});
            }
        }
        struct Row { int d; uint32_t i; float o, a, r; double v; };
        std::vector<Row> rows;
        std::set<int> dset;
        std::set<size_t> priced;
        {
            std::ifstream f(argv[4]); std::getline(f, line);
            while (std::getline(f, line)) {
                auto c = split(line);
                if (c.size() != 6) continue;
                auto it = sym_ix.find(c[1]);
                if (it == sym_ix.end()) continue;
                int d = iso(c[0]);
                dset.insert(d); priced.insert(it->second);
                rows.push_back({d, static_cast<uint32_t>(it->second), std::stof(c[2]), std::stof(c[3]), std::stof(c[4]), std::stod(c[5])});
            }
        }
        p.dates.assign(dset.begin(), dset.end());
        p.T = p.dates.size(); p.N = p.symbols.size();
        for (auto* a : {&p.open, &p.adj, &p.raw, &p.volume}) a->assign(p.T * p.N, kNaN);
        for (auto& r : rows) {
            size_t t = static_cast<size_t>(std::lower_bound(p.dates.begin(), p.dates.end(), r.d) - p.dates.begin());
            size_t x = p.ix(t, r.i);
            p.open[x] = r.o; p.adj[x] = r.a; p.raw[x] = r.r; p.volume[x] = r.v;
        }
        std::vector<Formation> log;
        auto lng = simulate_valprof(p, co, v, false, &log);
        auto opp = simulate_valprof(p, co, v, true);
        Evaluation strict = evaluate(lng, opp, {static_cast<int>(k.at("min_trades").num()), static_cast<int>(k.at("min_weeks").num())}, k.at("strict_alpha").num(), mult, seed);
        AdaptedResult ad = evaluate_active(lng, opp, k.at("adapted_alpha").num(), mult, seed, draws);
        std::map<long, double> monthly;
        for (auto& d : lng) monthly[month_key(d.date)] += d.gross - d.cost - d.bench;
        std::vector<double> mv; for (auto& [m, x] : monthly) mv.push_back(x);
        const double dsr = deflated_sharpe(mv, static_cast<int>(k.at("deflated_trials").num()));
        l.append("result", "{\"strict\":" + strict.json() + ",\"adapted_passed\":" + (ad.passed() ? "true" : "false") + ",\"deflated_sharpe\":" + std::to_string(dsr) + "}");

        std::printf("companies=%zu priced=%zu dates=%zu\n", p.N, priced.size(), p.T);
        std::printf("formations (date universe held orders):");
        for (auto& f : log) std::printf(" %d/%zu/%zu/%zu", f.date, f.universe, f.held, f.orders);
        std::printf("\n=== STRICT BAR (%s) ===\n", strict.passed() ? "PASS" : "FAIL");
        std::printf("days=%d orders=%d net=%+.3fbp/d excess=%+.3fbp/d excess_lb=%+.2fbp/wk alpha_lb=%+.2fbp/wk elevated=%+.3fbp/d worst_year=%+.3fbp/d opposite=%+.3fbp/d net_sharpe=%+.2f beta=%.2f total_net=%+.1f%% total_bench=%+.1f%%\n",
                    strict.days, strict.trades, strict.net_mean * 1e4, strict.excess_mean * 1e4, strict.excess_lb * 1e4, strict.alpha_lb * 1e4, strict.elevated_excess * 1e4, strict.worst_year * 1e4, strict.opposite_excess * 1e4, strict.sharpe, strict.beta, strict.total_net * 100, strict.total_bench * 100);
        for (auto& [g, ok] : strict.gates) std::printf("  %-42s %s\n", g.c_str(), ok ? "pass" : "FAIL");
        std::printf("=== ADAPTED BAR (%s) ===\n", ad.passed() ? "PASS" : "FAIL");
        std::printf("net=%+.3fbp/d doubled_cost=%+.3fbp/d active=%+.3fbp/d weekly_lb=%+.2fbp monthly_lb=%+.2fbp opp_active=%+.3fbp/d long_minus_opp_lb=%+.2fbp worst_year_active=%+.2f%% deflated_sharpe=%.3f\n",
                    ad.net_daily * 1e4, ad.elevated_daily * 1e4, ad.active_daily * 1e4, ad.weekly_lb * 1e4, ad.monthly_lb * 1e4, ad.opp_active_daily * 1e4, ad.long_minus_opp_weekly_lb * 1e4, ad.worst_year * 100, dsr);
        for (auto& [g, ok] : ad.gates) std::printf("  %-42s %s\n", g.c_str(), ok ? "pass" : "FAIL");
        std::map<int, double> yr, yb;
        for (auto& d : lng) { yr[d.date / 10000] += d.gross - d.cost; yb[d.date / 10000] += d.bench; }
        int ahead = 0; std::printf("calendar-year simple sums, net vs equal-weight universe:\n");
        for (auto& [y, x] : yr) { std::printf("  %d %+7.1f%% %+7.1f%%\n", y, x * 100, yb[y] * 100); ahead += x > yb[y]; }
        std::printf("years_ahead=%d/%zu\nknown_bias: %s\nexecution_authorized=false\n", ahead, yr.size(), k.at("known_bias").str().c_str());
        return 0;
    } catch (const std::exception& e) { std::cerr << "refused: " << e.what() << "\n"; return 1; }
}
