#pragma once
// The LLM boundary: proposals are data over a fixed vocabulary. There is no
// model-written code anywhere in the evaluation path.
#include <algorithm>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "json.hpp"
#include "panel.hpp"

namespace quant {

struct Term { int feature; double weight; };

struct Hypothesis {
    std::string id, family, model, market_filter;
    std::vector<Term> terms;
    int top_n = 10, rebalance_days = 5;
    std::string raw;

    // Same features and signs means the same idea, whatever the numeric knobs say.
    std::string signature() const {
        std::vector<std::string> parts;
        for (auto& t : terms) parts.push_back(std::string(t.weight > 0 ? "+" : "-") + kFeatureNames[t.feature]);
        std::sort(parts.begin(), parts.end());
        std::string s = model == "ridge_wf" ? "ridge:" : "linear:";
        for (auto& p : parts) s += p + ",";
        if (market_filter == "crash_1d") s += "|crash_1d";
        return s;
    }
};

inline void only_keys(const Json& j, std::set<std::string> allowed) {
    if (j.kind != Json::Obj) throw std::runtime_error("expected object");
    for (auto& [k, v] : j.o) if (!allowed.count(k)) throw std::runtime_error("unsupported field: " + k);
    for (auto& k : allowed) if (!j.has(k)) throw std::runtime_error("missing field: " + k);
}

inline Hypothesis parse_hypothesis(const std::string& text) {
    Json doc = parse_json(text);
    only_keys(doc, {"hypothesis", "rationale"});
    doc.at("rationale").str();
    const Json& h = doc.at("hypothesis");
    only_keys(h, {"schema", "id", "family", "model", "terms", "top_n", "rebalance_days", "market_filter"});
    if (h.at("schema").num() != 1) throw std::runtime_error("unsupported schema");
    Hypothesis out;
    out.id = h.at("id").str();
    out.family = h.at("family").str();
    out.model = h.at("model").str();
    out.market_filter = h.at("market_filter").str();
    out.top_n = static_cast<int>(h.at("top_n").num());
    out.rebalance_days = static_cast<int>(h.at("rebalance_days").num());
    if (out.id.empty() || out.id.size() > 48) throw std::runtime_error("bad id");
    if (out.model != "linear" && out.model != "ridge_wf") throw std::runtime_error("unsupported model");
    if (out.market_filter != "none" && out.market_filter != "breadth_5_20" && out.market_filter != "spy_above_200d" && out.market_filter != "crash_1d")
        throw std::runtime_error("unsupported market_filter");
    if (out.top_n != 5 && out.top_n != 10 && out.top_n != 20) throw std::runtime_error("top_n out of range");
    if (out.rebalance_days != 1 && out.rebalance_days != 5 && out.rebalance_days != 10)
        throw std::runtime_error("rebalance_days out of range");
    const Json& terms = h.at("terms");
    if (terms.kind != Json::Arr || terms.a.empty()) throw std::runtime_error("terms required");
    if (terms.a.size() > (out.model == "linear" ? 3u : 6u)) throw std::runtime_error("too many terms");
    std::set<int> seen;
    for (auto& t : terms.a) {
        only_keys(t, {"feature", "weight"});
        int f = feature_id(t.at("feature").str());
        double w = t.at("weight").num();
        if (f < 0) throw std::runtime_error("unsupported feature: " + t.at("feature").str());
        if (!seen.insert(f).second) throw std::runtime_error("duplicate feature");
        if (out.model == "linear" && w != 1 && w != -1 && w != 0.5 && w != -0.5)
            throw std::runtime_error("weight out of range");
        if (out.model == "ridge_wf" && w != 1) throw std::runtime_error("ridge terms must use weight 1");
        out.terms.push_back({f, w});
    }
    out.raw = doc.dump();
    return out;
}

// Refuses duplicates and near-duplicates before they can spend a trial.
class Admission {
public:
    Admission(int budget, int family_quota) : budget_(budget), quota_(family_quota) {}
    // Returns an empty string when admitted, otherwise the refusal reason.
    std::string consider(const Hypothesis& h) {
        if (trials_ >= budget_) return "trial_budget_exhausted";
        if (!signatures_.insert(h.signature()).second) return "near_duplicate";
        if (++family_[h.family] > quota_) return "family_quota";
        ++trials_;
        return "";
    }
    int trials() const { return trials_; }

private:
    int budget_, quota_, trials_ = 0;
    std::set<std::string> signatures_;
    std::map<std::string, int> family_;
};

}  // namespace quant
