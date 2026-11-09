#pragma once
// Portfolio construction and cost-aware simulation. A signal computed from data
// through the close of day s trades at the open of day s+1.
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

#include "hypothesis.hpp"
#include "panel.hpp"

namespace quant {

struct CostModel {
    double capital_usd = 2500;  // Fixed tickets make commission drag depend on account size.
    double slip_bps = 3;        // Half-spread plus impact, per side.
    double min_commission = 0.35;
    double per_share = 0.0035;
};

struct Day {
    int date = 0;
    double gross = 0, cost = 0, bench = 0;
    int tickets = 0;
    double exposure = 0;
};

struct Target {
    std::vector<size_t> names;
    bool in_market = false;
};

namespace detail {

inline std::vector<double> cross_zscore(const FeatureStore& fs, int f, size_t t) {
    std::vector<double> z(fs.N, kNaN);
    double sum = 0, sq = 0; size_t n = 0;
    for (size_t i = 0; i < fs.N; ++i) {
        double x = fs.at(f, t, i);
        if (std::isfinite(x)) { sum += x; sq += x * x; ++n; }
    }
    if (n < 2) return z;
    double mean = sum / n, var = sq / n - mean * mean;
    if (var <= 1e-18) return z;
    double sd = std::sqrt(var);
    for (size_t i = 0; i < fs.N; ++i) {
        double x = fs.at(f, t, i);
        if (std::isfinite(x)) z[i] = std::clamp((x - mean) / sd, -3.0, 3.0);
    }
    return z;
}

// Solves (A + lambda I) x = b by Gaussian elimination with partial pivoting.
inline std::vector<double> solve_ridge(std::vector<std::vector<double>> A, std::vector<double> b, double lambda) {
    size_t d = b.size();
    for (size_t i = 0; i < d; ++i) A[i][i] += lambda;
    for (size_t c = 0; c < d; ++c) {
        size_t piv = c;
        for (size_t r = c + 1; r < d; ++r) if (std::fabs(A[r][c]) > std::fabs(A[piv][c])) piv = r;
        std::swap(A[c], A[piv]); std::swap(b[c], b[piv]);
        for (size_t r = c + 1; r < d; ++r) {
            double m = A[r][c] / A[c][c];
            for (size_t k = c; k < d; ++k) A[r][k] -= m * A[c][k];
            b[r] -= m * b[c];
        }
    }
    std::vector<double> x(d);
    for (size_t c = d; c-- > 0;) {
        double s = b[c];
        for (size_t k = c + 1; k < d; ++k) s -= A[c][k] * x[k];
        x[c] = s / A[c][c];
    }
    return x;
}

}  // namespace detail

class Scorer {
public:
    Scorer(const Panel& p, const FeatureStore& fs, const Hypothesis& h) : p_(p), fs_(fs), h_(h) {
        coef_.resize(h.terms.size());
        for (size_t k = 0; k < h.terms.size(); ++k) coef_[k] = h.terms[k].weight;
    }

    std::vector<double> scores(size_t s) {
        if (h_.model == "ridge_wf" && (last_fit_ < 0 || static_cast<long>(s) - last_fit_ >= 21)) {
            fit(s);
            last_fit_ = static_cast<long>(s);
        }
        std::vector<double> out(p_.N, 0.0);
        std::vector<int> valid(p_.N, 1);
        for (size_t k = 0; k < h_.terms.size(); ++k) {
            auto z = detail::cross_zscore(fs_, h_.terms[k].feature, s);
            for (size_t i = 0; i < p_.N; ++i) {
                if (!std::isfinite(z[i])) valid[i] = 0;
                else out[i] += coef_[k] * z[i];
            }
        }
        for (size_t i = 0; i < p_.N; ++i) if (!valid[i]) out[i] = kNaN;
        return out;
    }

    const std::vector<double>& coefficients() const { return coef_; }

private:
    // Walk-forward ridge. Every label window closes at or before the open of day s,
    // so a training row cannot contain information from after the decision time.
    void fit(size_t s) {
        const size_t d = h_.terms.size();
        const size_t horizon = static_cast<size_t>(h_.rebalance_days);
        std::vector<std::vector<double>> xtx(d, std::vector<double>(d, 0.0));
        std::vector<double> xty(d, 0.0);
        size_t rows = 0;
        const size_t lookback = 504;
        if (s < horizon + 2) { std::fill(coef_.begin(), coef_.end(), 0.0); return; }
        size_t last_tau = s - horizon - 1;
        size_t first_tau = last_tau > lookback ? last_tau - lookback : 0;
        for (size_t tau = first_tau; tau <= last_tau; tau += 5) {
            std::vector<std::vector<double>> z(d);
            for (size_t k = 0; k < d; ++k) z[k] = detail::cross_zscore(fs_, h_.terms[k].feature, tau);
            std::vector<double> y(p_.N, kNaN);
            double sum = 0; size_t n = 0;
            for (size_t i = 0; i < p_.N; ++i) {
                double a = p_.open[p_.ix(tau + 1, i)], b = p_.open[p_.ix(tau + 1 + horizon, i)];
                if (a > 0 && b > 0) { y[i] = b / a - 1.0; sum += y[i]; ++n; }
            }
            if (n < 2) continue;
            double mean = sum / n;
            for (size_t i = 0; i < p_.N; ++i) {
                bool ok = std::isfinite(y[i]);
                for (size_t k = 0; k < d && ok; ++k) ok = std::isfinite(z[k][i]);
                if (!ok) continue;
                for (size_t a = 0; a < d; ++a) {
                    xty[a] += z[a][i] * (y[i] - mean);
                    for (size_t b = 0; b < d; ++b) xtx[a][b] += z[a][i] * z[b][i];
                }
                ++rows;
            }
        }
        if (rows < 500) { std::fill(coef_.begin(), coef_.end(), 0.0); return; }
        coef_ = detail::solve_ridge(xtx, xty, 0.05 * static_cast<double>(rows));
    }

    const Panel& p_;
    const FeatureStore& fs_;
    const Hypothesis& h_;
    std::vector<double> coef_;
    long last_fit_ = -1;
};

inline bool market_allows(const FeatureStore& fs, const Panel& p, const std::string& filter, size_t s) {
    if (filter == "breadth_5_20") return fs.breadth5[s] > 0 && fs.breadth20[s] > 0;
    if (filter == "spy_above_200d") return p.spy_close[s] > fs.spy_ma200[s];
    if (filter == "crash_1d") return fs.breadth1[s] <= -0.02;
    return true;
}

inline Target select_target(const Panel& p, const FeatureStore& fs, const Hypothesis& h, Scorer& scorer, size_t s, bool invert) {
    Target t;
    t.in_market = market_allows(fs, p, h.market_filter, s);
    if (!t.in_market) return t;
    auto sc = scorer.scores(s);
    std::vector<size_t> idx;
    for (size_t i = 0; i < p.N; ++i) if (std::isfinite(sc[i])) idx.push_back(i);
    if (idx.size() < static_cast<size_t>(h.top_n) * 2) { t.in_market = false; return t; }
    std::sort(idx.begin(), idx.end(), [&](size_t a, size_t b) {
        if (sc[a] != sc[b]) return invert ? sc[a] < sc[b] : sc[a] > sc[b];
        return a < b;
    });
    idx.resize(static_cast<size_t>(h.top_n));
    t.names = std::move(idx);
    return t;
}

// Simulates executions dated within [from, to]. With mark_last the final row marks
// open-to-close, used only for forward accounting when no next open exists yet.
inline std::vector<Day> simulate(const Panel& p, const FeatureStore& fs, const Hypothesis& h, const CostModel& c,
                                 int from, int to, bool invert = false, bool mark_last = false) {
    std::vector<Day> out;
    std::vector<double> w(p.N, 0.0);
    Scorer scorer(p, fs, h);
    long first = -1;
    for (size_t e = 1; e < p.T; ++e) {
        if (p.dates[e] < from) continue;
        if (p.dates[e] > to) break;
        const size_t s = e - 1;
        if (e + 1 >= p.T && !mark_last) break;
        if (first < 0) first = static_cast<long>(e);
        Day d;
        d.date = p.dates[e];
        if ((static_cast<long>(e) - first) % h.rebalance_days == 0) {
            Target t = select_target(p, fs, h, scorer, s, invert);
            std::vector<double> target(p.N, 0.0);
            for (size_t i : t.names) target[i] = 1.0 / static_cast<double>(h.top_n);
            for (size_t i = 0; i < p.N; ++i) {
                double dw = std::fabs(target[i] - w[i]);
                if (dw < 1e-9) continue;
                double px = p.open[p.ix(e, i)];
                if (!(px > 0)) { target[i] = w[i]; continue; }  // Unpriced names cannot trade today.
                double ticket = dw * c.capital_usd;
                double shares = ticket / px;
                double commission = std::min(0.01 * ticket, std::max(c.min_commission, c.per_share * shares));
                d.cost += commission / c.capital_usd + dw * c.slip_bps * 1e-4;
                ++d.tickets;
            }
            w = target;
        }
        double bench_sum = 0; size_t bench_n = 0;
        std::vector<double> r(p.N, 0.0);
        for (size_t i = 0; i < p.N; ++i) {
            double a = p.open[p.ix(e, i)];
            double b = e + 1 < p.T ? p.open[p.ix(e + 1, i)] : p.close[p.ix(e, i)];
            if (a > 0 && b > 0) { r[i] = b / a - 1.0; bench_sum += r[i]; ++bench_n; }
            d.gross += w[i] * r[i];
            d.exposure += w[i];
        }
        d.bench = bench_n ? bench_sum / bench_n : 0.0;
        for (size_t i = 0; i < p.N; ++i) w[i] = w[i] * (1 + r[i]) / (1 + d.gross);
        out.push_back(d);
    }
    return out;
}

}  // namespace quant
