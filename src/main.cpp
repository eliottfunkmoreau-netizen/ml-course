// quant: bounded research loop, portfolio construction and simulated execution.
#include <atomic>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <random>
#include <thread>

#include "quant/gates.hpp"
#include "quant/ledger.hpp"

using namespace quant;
namespace fs_ = std::filesystem;

namespace {

struct Stage { int from, to; Floors floors; };

struct Contract {
    Stage discovery, validation, holdout;
    CostModel cost;
    double elevated_mult;
    int trial_budget, family_quota, shortlist_max;
    uint64_t seed;
    bool active_days_only = false;
    int min_year_days = 40;
};

Stage parse_stage(const Json& c, const char* name) {
    const Json& w = c.at("stages").at(name);
    return {date_to_int(w.at("from").str()), date_to_int(w.at("to").str()),
            {static_cast<int>(w.at("min_trades").num()), static_cast<int>(w.at("min_weeks").num())}};
}

Contract parse_contract(const Json& c) {
    Contract k;
    k.discovery = parse_stage(c, "discovery");
    k.validation = parse_stage(c, "validation");
    k.holdout = parse_stage(c, "holdout");
    k.cost.capital_usd = c.at("capital_usd").num();
    k.cost.slip_bps = c.at("slip_bps").num();
    k.elevated_mult = c.at("elevated_mult").num();
    k.trial_budget = static_cast<int>(c.at("trial_budget").num());
    k.family_quota = static_cast<int>(c.at("family_quota").num());
    k.shortlist_max = static_cast<int>(c.at("shortlist_max").num());
    k.seed = static_cast<uint64_t>(c.at("seed").num());
    if (c.has("active_days_only")) k.active_days_only = c.at("active_days_only").b;
    if (c.has("min_year_days")) k.min_year_days = static_cast<int>(c.at("min_year_days").num());
    return k;
}

struct Run {
    std::string dir;
    Ledger ledger;
    Json freeze;
    Contract contract;
    std::string panel_path;

    explicit Run(const std::string& d) : dir(d), ledger(d + "/ledger.tsv") {
        auto* f = ledger.last_of("freeze");
        if (!f) throw std::runtime_error("run is not frozen");
        freeze = parse_json(f->payload);
        contract = parse_contract(freeze.at("contract"));
        panel_path = freeze.at("panel_path").str();
    }
    Panel frozen_panel() const {
        if (sha256(read_file(panel_path)) != freeze.at("panel_sha256").str())
            throw std::runtime_error("panel does not match the frozen hash; refusing to evaluate");
        return load_panel(panel_path);
    }
    Hypothesis hypothesis(const std::string& id) const {
        for (auto& e : ledger.events())
            if (e.type == "admitted" && parse_json(e.payload).at("id").str() == id)
                return parse_hypothesis(parse_json(e.payload).at("proposal").dump());
        throw std::runtime_error("no admitted hypothesis " + id);
    }
};

std::vector<std::string> json_ids(const Json& j) {
    std::vector<std::string> out;
    for (auto& x : j.at("ids").a) out.push_back(x.str());
    return out;
}

std::string ids_json(const std::vector<std::string>& ids) {
    Json a; a.kind = Json::Arr;
    for (auto& id : ids) { Json s; s.kind = Json::Str; s.s = id; a.a.push_back(s); }
    return "{\"ids\":" + a.dump() + "}";
}

Evaluation run_stage(const Panel& p, const FeatureStore& fs, const Hypothesis& h, const Stage& st, const Contract& k, double alpha) {
    auto days = simulate(p, fs, h, k.cost, st.from, st.to);
    auto opp = simulate(p, fs, h, k.cost, st.from, st.to, true);
    if (k.active_days_only) { days = active_days(days); opp = active_days(opp); }
    return evaluate(days, opp, st.floors, alpha, k.elevated_mult, k.seed, k.min_year_days);
}

std::string result_payload(const std::string& id, const Evaluation& ev) {
    return "{\"id\":" + Json::quote(id) + ",\"evaluation\":" + ev.json() + "}";
}

int cmd_freeze(const std::string& dir, const std::string& contract_path, const std::string& panel_path, const std::string& proposals) {
    fs_::create_directories(dir);
    if (fs_::exists(dir + "/ledger.tsv")) throw std::runtime_error("run already frozen; a changed contract needs a new run");
    Json contract = parse_json(read_file(contract_path));
    parse_contract(contract);
    std::vector<std::string> files;
    for (auto& e : fs_::directory_iterator(proposals)) if (e.path().extension() == ".json") files.push_back(e.path().string());
    std::sort(files.begin(), files.end());
    std::string all;
    for (auto& f : files) all += sha256(read_file(f));
    Ledger l(dir + "/ledger.tsv");
    l.append("freeze", "{\"contract\":" + contract.dump() + ",\"panel_path\":" + Json::quote(fs_::absolute(panel_path).string()) +
                           ",\"panel_sha256\":" + Json::quote(sha256(read_file(panel_path))) +
                           ",\"proposals_dir\":" + Json::quote(fs_::absolute(proposals).string()) +
                           ",\"proposals_sha256\":" + Json::quote(sha256(all)) + "}");
    std::cout << "frozen: " << l.events().back().hash << " (" << files.size() << " proposals)\n";
    return 0;
}

int cmd_discover(const std::string& dir) {
    Run r(dir);
    if (r.ledger.last_of("shortlist")) throw std::runtime_error("discovery already closed for this run");
    const std::string proposals = r.freeze.at("proposals_dir").str();
    std::vector<std::string> files;
    for (auto& e : fs_::directory_iterator(proposals)) if (e.path().extension() == ".json") files.push_back(e.path().string());
    std::sort(files.begin(), files.end());
    std::string all;
    for (auto& f : files) all += sha256(read_file(f));
    if (sha256(all) != r.freeze.at("proposals_sha256").str()) throw std::runtime_error("proposals changed after freeze");

    const Contract& k = r.contract;
    Panel p = r.frozen_panel().up_to(k.discovery.to);
    FeatureStore fs = compute_features(p);
    Admission adm(k.trial_budget, k.family_quota);
    struct Passed { std::string id, family; double score; };
    std::vector<Passed> passed;
    for (auto& f : files) {
        Hypothesis h;
        try {
            h = parse_hypothesis(read_file(f));
        } catch (const std::exception& e) {
            r.ledger.append("invalid", "{\"file\":" + Json::quote(fs_::path(f).filename().string()) + ",\"reason\":" + Json::quote(e.what()) + "}");
            std::printf("%-26s INVALID  %s\n", fs_::path(f).filename().c_str(), e.what());
            continue;
        }
        std::string refusal = adm.consider(h);
        if (!refusal.empty()) {
            r.ledger.append("refused", "{\"id\":" + Json::quote(h.id) + ",\"reason\":" + Json::quote(refusal) + "}");
            std::printf("%-26s REFUSED  %s\n", h.id.c_str(), refusal.c_str());
            continue;
        }
        r.ledger.append("admitted", "{\"id\":" + Json::quote(h.id) + ",\"proposal\":" + h.raw + "}");
        Evaluation ev = run_stage(p, fs, h, k.discovery, k, 0.05);
        r.ledger.append("discovery_trial", result_payload(h.id, ev));
        std::printf("%-26s %s excess=%+7.2fbp/d lb=%+7.2fbp/wk net_sharpe=%+5.2f trades=%d", h.id.c_str(), ev.passed() ? "PASS" : "fail",
                    ev.excess_mean * 1e4, ev.excess_lb * 1e4, ev.sharpe, ev.trades);
        for (auto& [g, ok] : ev.gates) if (!ok) std::printf(" !%s", g.c_str());
        std::printf("\n");
        if (ev.passed()) passed.push_back({h.id, h.family, ev.rank_score()});
    }
    std::sort(passed.begin(), passed.end(), [](auto& a, auto& b) { return a.score > b.score; });
    std::vector<std::string> shortlist;
    std::set<std::string> families;
    for (auto& c : passed)
        if (static_cast<int>(shortlist.size()) < k.shortlist_max && families.insert(c.family).second) shortlist.push_back(c.id);
    r.ledger.append("shortlist", ids_json(shortlist));
    std::cout << "trials=" << adm.trials() << " passed=" << passed.size() << " shortlist=" << shortlist.size() << "\n";
    return 0;
}

int cmd_validate(const std::string& dir) {
    Run r(dir);
    auto* sl = r.ledger.last_of("shortlist");
    if (!sl) throw std::runtime_error("run discovery first");
    if (r.ledger.last_of("validation_closed")) throw std::runtime_error("validation already closed for this run");
    auto ids = json_ids(parse_json(sl->payload));
    const Contract& k = r.contract;
    if (ids.empty()) {
        r.ledger.append("validation_closed", "{\"holdout_ready\":null,\"reason\":\"empty_shortlist\"}");
        std::cout << "shortlist empty: validation left unread, nothing promoted\n";
        return 0;
    }
    Panel p = r.frozen_panel().up_to(k.validation.to);
    FeatureStore fs = compute_features(p);
    const double alpha = 0.05 / static_cast<double>(ids.size());
    std::string best; double best_score = -1e18;
    for (auto& id : ids) {
        Hypothesis h = r.hypothesis(id);
        Evaluation ev = run_stage(p, fs, h, k.validation, k, alpha);
        r.ledger.append("validation_trial", result_payload(id, ev));
        std::printf("%-26s %s excess=%+7.2fbp/d lb=%+7.2fbp/wk net_sharpe=%+5.2f trades=%d", id.c_str(), ev.passed() ? "PASS" : "fail",
                    ev.excess_mean * 1e4, ev.excess_lb * 1e4, ev.sharpe, ev.trades);
        for (auto& [g, ok] : ev.gates) if (!ok) std::printf(" !%s", g.c_str());
        std::printf("\n");
        if (ev.passed() && ev.rank_score() > best_score) { best_score = ev.rank_score(); best = id; }
    }
    r.ledger.append("validation_closed", best.empty() ? "{\"holdout_ready\":null,\"reason\":\"no_survivor\"}" : "{\"holdout_ready\":" + Json::quote(best) + "}");
    std::cout << (best.empty() ? std::string("no survivor: holdout stays sealed\n") : "holdout_ready: " + best + "\n");
    return 0;
}

int cmd_holdout(const std::string& dir) {
    Run r(dir);
    auto* vc = r.ledger.last_of("validation_closed");
    if (!vc) throw std::runtime_error("run validation first");
    Json state = parse_json(vc->payload);
    if (state.at("holdout_ready").kind != Json::Str) throw std::runtime_error("no candidate is holdout_ready");
    const std::string id = state.at("holdout_ready").str();
    if (!claim_receipt(dir + "/holdout.receipt", id + "\n")) throw std::runtime_error("holdout already spent for this run");
    r.ledger.append("holdout_receipt", "{\"id\":" + Json::quote(id) + "}");
    const Contract& k = r.contract;
    Panel p = r.frozen_panel().up_to(k.holdout.to);
    FeatureStore fs = compute_features(p);
    Hypothesis h = r.hypothesis(id);
    Evaluation ev = run_stage(p, fs, h, k.holdout, k, 0.05);
    r.ledger.append("holdout_result", result_payload(id, ev));
    std::cout << ev.json() << "\n";
    r.ledger.append("finding", "{\"id\":" + Json::quote(id) + ",\"promoted\":" + (ev.passed() ? "true" : "false") + ",\"authority\":\"research_only\",\"execution_authorized\":false}");
    std::cout << (ev.passed() ? "PROMOTED to simulated forward test: " : "REJECTED on holdout: ") << id << "\n";
    return 0;
}

std::string promoted_or_shadow(Run& r, int argc, char** argv, int at, bool& shadow) {
    if (argc > at + 1 && std::string(argv[at]) == "--shadow") {
        shadow = true;
        return argv[at + 1];
    }
    auto* f = r.ledger.last_of("finding");
    if (!f || !parse_json(f->payload).at("promoted").b)
        throw std::runtime_error("no promoted strategy; nothing may trade (use --shadow <id> for a labelled diagnostic)");
    return parse_json(f->payload).at("id").str();
}

int cmd_forward(int argc, char** argv) {
    Run r(argv[2]);
    bool shadow = false;
    std::string id = promoted_or_shadow(r, argc, argv, 6, shadow);
    Panel p = load_panel(argv[3]);
    FeatureStore fs = compute_features(p);
    Hypothesis h = r.hypothesis(id);
    int from = date_to_int(argv[4]), to = date_to_int(argv[5]);
    auto days = simulate(p, fs, h, r.contract.cost, from, to, false, true);
    double nav = 1, bench = 1;
    std::printf("%s forward simulation of %s (execution_authorized=false)\n", shadow ? "SHADOW" : "PROMOTED", id.c_str());
    for (auto& d : days) {
        double net = d.gross - d.cost;
        nav *= 1 + net; bench *= 1 + d.bench;
        std::printf("%d net=%+7.3f%% cost=%6.3f%% tickets=%2d exposure=%4.2f nav=%.4f equal_weight=%.4f\n", d.date, net * 100, d.cost * 100, d.tickets, d.exposure, nav, bench);
    }
    std::printf("capital_usd=%.0f pnl_usd=%+.2f bench_usd=%+.2f\n", r.contract.cost.capital_usd, (nav - 1) * r.contract.cost.capital_usd, (bench - 1) * r.contract.cost.capital_usd);
    return 0;
}

int cmd_target(int argc, char** argv) {
    Run r(argv[2]);
    bool shadow = false;
    std::string id = promoted_or_shadow(r, argc, argv, 4, shadow);
    Panel p = load_panel(argv[3]);
    FeatureStore fs = compute_features(p);
    Hypothesis h = r.hypothesis(id);
    Scorer scorer(p, fs, h);
    Target t = select_target(p, fs, h, scorer, p.T - 1, false);
    std::printf("%s target for %s, signal date %d, in_market=%d\n", shadow ? "SHADOW" : "PROMOTED", id.c_str(), p.dates.back(), t.in_market);
    for (size_t i : t.names) std::printf("  %-6s weight=%.3f notional_usd=%.2f\n", p.symbols[i].c_str(), 1.0 / h.top_n, r.contract.cost.capital_usd / h.top_n);
    return 0;
}

// Measures the in-process path only: tick dequeued -> book updated -> risk check -> order intent.
int cmd_bench() {
    constexpr size_t kCap = 1 << 16, kTicks = 2'000'000, kSyms = 100;
    struct Tick { uint32_t sym; float px; int64_t t0; };
    alignas(64) std::atomic<size_t> head{0};
    alignas(64) std::atomic<size_t> tail{0};
    std::vector<Tick> ring(kCap);
    auto now = [] { return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count(); };
    std::vector<int64_t> lat;
    lat.reserve(kTicks);
    std::thread consumer([&] {
        std::array<double, kSyms> last{}, target{};
        target.fill(25.0);
        size_t done = 0, orders = 0;
        while (done < kTicks) {
            size_t t = tail.load(std::memory_order_relaxed);
            if (t == head.load(std::memory_order_acquire)) continue;
            Tick k = ring[t & (kCap - 1)];
            tail.store(t + 1, std::memory_order_release);
            last[k.sym] = k.px;
            double notional = target[k.sym] * k.px;
            if (notional > 2600.0 && notional < 100000.0) ++orders;  // Position-limit check before an intent.
            lat.push_back(now() - k.t0);
            ++done;
        }
        std::printf("order_intents=%zu\n", orders);
    });
    std::mt19937 rng(7);
    for (size_t i = 0; i < kTicks; ++i) {
        size_t h = head.load(std::memory_order_relaxed);
        while (h - tail.load(std::memory_order_acquire) >= kCap) {}
        ring[h & (kCap - 1)] = Tick{static_cast<uint32_t>(rng() % kSyms), 90.0f + static_cast<float>(rng() % 2000) * 0.01f, now()};
        head.store(h + 1, std::memory_order_release);
        for (int w = 0; w < 200; ++w) std::atomic_signal_fence(std::memory_order_seq_cst);  // Pace the feed so queueing does not dominate.
    }
    consumer.join();
    std::sort(lat.begin(), lat.end());
    auto q = [&](double f) { return lat[static_cast<size_t>(f * static_cast<double>(lat.size() - 1))]; };
    std::printf("ticks=%zu p50=%lldns p99=%lldns p99.9=%lldns max=%lldns\n", lat.size(), (long long)q(.5), (long long)q(.99), (long long)q(.999), (long long)lat.back());
    return 0;
}

// Diagnostic only: forward excess return by feature decile. It cannot promote anything.
int cmd_deciles(const std::string& panel, const std::string& feature, int from, int to, int horizon) {
    const int f = feature_id(feature);
    if (f < 0) throw std::runtime_error("unknown feature");
    Panel p = load_panel(panel).up_to(to);
    FeatureStore fs = compute_features(p);
    std::array<double, 10> sum{};
    std::array<int, 10> dates{};
    std::vector<double> ics;
    for (size_t s = 130; s + 1 + static_cast<size_t>(horizon) < p.T; s += static_cast<size_t>(horizon)) {
        if (p.dates[s + 1] < from) continue;
        std::vector<std::pair<double, double>> xs;
        double mean = 0;
        for (size_t i = 0; i < p.N; ++i) {
            double x = fs.at(f, s, i), a = p.open[p.ix(s + 1, i)], b = p.open[p.ix(s + 1 + static_cast<size_t>(horizon), i)];
            if (std::isfinite(x) && a > 0 && b > 0) { xs.emplace_back(x, b / a - 1); mean += b / a - 1; }
        }
        if (xs.size() < 50) continue;
        mean /= static_cast<double>(xs.size());
        std::sort(xs.begin(), xs.end());
        const double n = static_cast<double>(xs.size());
        std::array<double, 10> dec{};
        std::array<int, 10> cnt{};
        std::vector<std::pair<double, size_t>> byret;
        for (size_t k = 0; k < xs.size(); ++k) { size_t d = std::min<size_t>(9, k * 10 / xs.size()); dec[d] += xs[k].second - mean; ++cnt[d]; byret.emplace_back(xs[k].second, k); }
        for (size_t d = 0; d < 10; ++d) if (cnt[d]) { sum[d] += dec[d] / cnt[d]; ++dates[d]; }
        std::sort(byret.begin(), byret.end());
        double d2 = 0;
        for (size_t r = 0; r < byret.size(); ++r) { double diff = static_cast<double>(r) - static_cast<double>(byret[r].second); d2 += diff * diff; }
        ics.push_back(1 - 6 * d2 / (n * (n * n - 1)));
    }
    std::printf("%s deciles, forward %d-day open-to-open excess (bp), DIAGNOSTIC ONLY, %d periods\n", feature.c_str(), horizon, dates[0]);
    for (size_t d = 0; d < 10; ++d) std::printf("  D%zu%s %+7.1f\n", d + 1, d == 0 ? " (lowest)" : d == 9 ? " (highest)" : "", sum[d] / dates[d] * 1e4);
    double m = 0, v = 0;
    for (double c : ics) m += c;
    m /= static_cast<double>(ics.size());
    for (double c : ics) v += (c - m) * (c - m);
    v /= static_cast<double>(ics.size() - 1);
    std::printf("  mean rank IC %+.4f  t=%+.2f  (top minus bottom decile %+.1f bp)\n", m, m / std::sqrt(v / static_cast<double>(ics.size())), (sum[9] / dates[9] - sum[0] / dates[0]) * 1e4);
    return 0;
}

int cmd_verify(const std::string& dir) {
    Ledger l(dir + "/ledger.tsv");
    std::cout << "ledger OK: " << l.events().size() << " records, head " << (l.events().empty() ? "-" : l.events().back().hash) << "\n";
    return 0;
}

int usage() {
    std::cerr << "usage: quant freeze <run> <contract> <panel> <proposals_dir>\n"
                 "       quant discover|validate|holdout|verify <run>\n"
                 "       quant target <run> <panel> [--shadow id]\n"
                 "       quant forward <run> <panel> <from> <to> [--shadow id]\n"
                 "       quant bench\n";
    return 2;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc < 2) return usage();
        const std::string c = argv[1];
        if (c == "bench") return cmd_bench();
        if (c == "freeze" && argc == 6) return cmd_freeze(argv[2], argv[3], argv[4], argv[5]);
        if (c == "discover" && argc == 3) return cmd_discover(argv[2]);
        if (c == "validate" && argc == 3) return cmd_validate(argv[2]);
        if (c == "holdout" && argc == 3) return cmd_holdout(argv[2]);
        if (c == "deciles" && argc == 7) return cmd_deciles(argv[2], argv[3], date_to_int(argv[4]), date_to_int(argv[5]), std::stoi(argv[6]));
        if (c == "verify" && argc == 3) return cmd_verify(argv[2]);
        if (c == "target" && argc >= 4) return cmd_target(argc, argv);
        if (c == "forward" && argc >= 6) return cmd_forward(argc, argv);
        return usage();
    } catch (const std::exception& e) {
        std::cerr << "refused: " << e.what() << "\n";
        return 1;
    }
}
