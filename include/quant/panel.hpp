#pragma once
// Rectangular T x N market panel and strictly causal features: every feature at
// row t reads only rows <= t.
#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace quant {

inline const double kNaN = std::numeric_limits<double>::quiet_NaN();

struct Panel {
    std::vector<int> dates;
    std::vector<std::string> symbols;
    size_t T = 0, N = 0;
    std::vector<double> open, high, low, close, volume, fshort, ftotal, spy_close;

    size_t ix(size_t t, size_t i) const { return t * N + i; }

    void allocate(size_t t, size_t n) {
        T = t; N = n;
        for (auto* v : {&open, &high, &low, &close, &volume, &fshort, &ftotal}) v->assign(T * N, kNaN);
        spy_close.assign(T, kNaN);
    }

    // Physically drops every row after `last_date`, so later evidence is unreachable.
    Panel up_to(int last_date) const {
        size_t keep = std::upper_bound(dates.begin(), dates.end(), last_date) - dates.begin();
        Panel p;
        p.dates.assign(dates.begin(), dates.begin() + keep);
        p.symbols = symbols;
        p.T = keep; p.N = N;
        auto cut = [&](const std::vector<double>& src, std::vector<double>& dst) {
            dst.assign(src.begin(), src.begin() + keep * N);
        };
        cut(open, p.open); cut(high, p.high); cut(low, p.low); cut(close, p.close);
        cut(volume, p.volume); cut(fshort, p.fshort); cut(ftotal, p.ftotal);
        p.spy_close.assign(spy_close.begin(), spy_close.begin() + keep);
        return p;
    }
};

inline int date_to_int(const std::string& d) {
    if (d.size() != 10) throw std::runtime_error("bad date " + d);
    return std::stoi(d.substr(0, 4)) * 10000 + std::stoi(d.substr(5, 2)) * 100 + std::stoi(d.substr(8, 2));
}

inline Panel load_panel(const std::string& path, const std::string& bench = "SPY") {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("cannot open " + path);
    struct Row { int date; std::string sym; double v[7]; };
    std::vector<Row> rows;
    std::set<int> dset;
    std::set<std::string> sset;
    std::string line;
    std::getline(f, line);
    while (std::getline(f, line)) {
        std::stringstream ss(line);
        std::string cell;
        Row r;
        std::getline(ss, cell, ','); r.date = date_to_int(cell);
        std::getline(ss, r.sym, ',');
        for (double& x : r.v) { std::getline(ss, cell, ','); x = std::stod(cell); }
        dset.insert(r.date);
        if (r.sym != bench) sset.insert(r.sym);
        rows.push_back(std::move(r));
    }
    Panel p;
    p.dates.assign(dset.begin(), dset.end());
    p.symbols.assign(sset.begin(), sset.end());
    p.allocate(p.dates.size(), p.symbols.size());
    std::map<int, size_t> dpos;
    for (size_t i = 0; i < p.dates.size(); ++i) dpos[p.dates[i]] = i;
    std::map<std::string, size_t> spos;
    for (size_t i = 0; i < p.symbols.size(); ++i) spos[p.symbols[i]] = i;
    for (auto& r : rows) {
        size_t t = dpos[r.date];
        if (r.sym == bench) { p.spy_close[t] = r.v[3]; continue; }
        size_t k = p.ix(t, spos[r.sym]);
        p.open[k] = r.v[0]; p.high[k] = r.v[1]; p.low[k] = r.v[2]; p.close[k] = r.v[3];
        p.volume[k] = r.v[4]; p.fshort[k] = r.v[5]; p.ftotal[k] = r.v[6];
    }
    return p;
}

enum Feature {
    RET_1, RET_5, RET_20, RET_60, RET_120, VOL_20, RANGE_20, DOLLAR_VOL_20,
    OFF_SHARE_5, OFF_SHARE_20, OFF_SHARE_CHG, SHORT_RATIO_5, SHORT_RATIO_20, kFeatureCount
};

inline const std::array<const char*, kFeatureCount> kFeatureNames = {
    "ret_1", "ret_5", "ret_20", "ret_60", "ret_120", "vol_20", "range_20", "dollar_vol_20",
    "off_share_5", "off_share_20", "off_share_chg", "short_ratio_5", "short_ratio_20"};

inline int feature_id(const std::string& n) {
    for (int i = 0; i < kFeatureCount; ++i) if (n == kFeatureNames[i]) return i;
    return -1;
}

struct FeatureStore {
    size_t T = 0, N = 0;
    std::vector<double> v;
    std::vector<double> breadth1, breadth5, breadth20, spy_ma200;
    double at(int f, size_t t, size_t i) const { return v[(static_cast<size_t>(f) * T + t) * N + i]; }
    double& at(int f, size_t t, size_t i) { return v[(static_cast<size_t>(f) * T + t) * N + i]; }
};

namespace detail {
// Mean over the trailing `w` rows ending at t; NaN unless 80% of the window is present.
template <class Fn>
double trailing_mean(size_t t, size_t w, Fn value) {
    if (t + 1 < w) return kNaN;
    double sum = 0; size_t n = 0;
    for (size_t k = t + 1 - w; k <= t; ++k) {
        double x = value(k);
        if (std::isfinite(x)) { sum += x; ++n; }
    }
    return n * 5 >= w * 4 ? sum / n : kNaN;
}
inline double median(std::vector<double>& x) {
    if (x.empty()) return kNaN;
    auto m = x.begin() + x.size() / 2;
    std::nth_element(x.begin(), m, x.end());
    return *m;
}
}  // namespace detail

inline FeatureStore compute_features(const Panel& p) {
    FeatureStore fs;
    fs.T = p.T; fs.N = p.N;
    fs.v.assign(static_cast<size_t>(kFeatureCount) * p.T * p.N, kNaN);
    const std::array<std::pair<int, size_t>, 5> rets{{{RET_1,1},{RET_5,5},{RET_20,20},{RET_60,60},{RET_120,120}}};
    for (size_t i = 0; i < p.N; ++i) {
        auto c = [&](size_t t) { return p.close[p.ix(t, i)]; };
        auto off = [&](size_t t) {
            double ft = p.ftotal[p.ix(t, i)], v = p.volume[p.ix(t, i)];
            return ft > 0 && v > 0 ? ft / v : kNaN;
        };
        auto sr = [&](size_t t) {
            double ft = p.ftotal[p.ix(t, i)];
            return ft > 0 ? p.fshort[p.ix(t, i)] / ft : kNaN;
        };
        for (size_t t = 0; t < p.T; ++t) {
            for (auto [f, k] : rets)
                if (t >= k) fs.at(f, t, i) = c(t) / c(t - k) - 1.0;
            if (t >= 20) {
                double m = detail::trailing_mean(t, 20, [&](size_t u) { return u ? std::log(c(u) / c(u-1)) : kNaN; });
                double ss = detail::trailing_mean(t, 20, [&](size_t u) {
                    double r = u ? std::log(c(u) / c(u-1)) : kNaN;
                    return (r - m) * (r - m);
                });
                fs.at(VOL_20, t, i) = std::sqrt(ss);
            }
            fs.at(RANGE_20, t, i) = detail::trailing_mean(t, 20, [&](size_t u) {
                return (p.high[p.ix(u,i)] - p.low[p.ix(u,i)]) / p.close[p.ix(u,i)]; });
            double dv = detail::trailing_mean(t, 20, [&](size_t u) { return p.close[p.ix(u,i)] * p.volume[p.ix(u,i)]; });
            if (dv > 0) fs.at(DOLLAR_VOL_20, t, i) = std::log(dv);
            double o5 = detail::trailing_mean(t, 5, off), o20 = detail::trailing_mean(t, 20, off);
            fs.at(OFF_SHARE_5, t, i) = o5;
            fs.at(OFF_SHARE_20, t, i) = o20;
            fs.at(OFF_SHARE_CHG, t, i) = o5 - o20;
            fs.at(SHORT_RATIO_5, t, i) = detail::trailing_mean(t, 5, sr);
            fs.at(SHORT_RATIO_20, t, i) = detail::trailing_mean(t, 20, sr);
        }
    }
    fs.breadth1.assign(p.T, kNaN); fs.breadth5.assign(p.T, kNaN); fs.breadth20.assign(p.T, kNaN); fs.spy_ma200.assign(p.T, kNaN);
    for (size_t t = 0; t < p.T; ++t) {
        std::vector<double> r1, r5, r20;
        for (size_t i = 0; i < p.N; ++i) {
            if (std::isfinite(fs.at(RET_1, t, i))) r1.push_back(fs.at(RET_1, t, i));
            if (std::isfinite(fs.at(RET_5, t, i))) r5.push_back(fs.at(RET_5, t, i));
            if (std::isfinite(fs.at(RET_20, t, i))) r20.push_back(fs.at(RET_20, t, i));
        }
        fs.breadth1[t] = detail::median(r1);
        fs.breadth5[t] = detail::median(r5);
        fs.breadth20[t] = detail::median(r20);
        fs.spy_ma200[t] = detail::trailing_mean(t, 200, [&](size_t u) { return p.spy_close[u]; });
    }
    return fs;
}

}  // namespace quant
