#!/usr/bin/env python3
"""Builds data/panel_recent.csv. Candidates are ranked using October 2024 data only,
so selection cannot use later outcomes. Symbols Yahoo no longer serves (typically
delisted) are counted and reported, because they are a remaining survivorship bias."""
import collections, csv, datetime as dt, json, pathlib, statistics as st, time
from concurrent.futures import ThreadPoolExecutor
from fetch_data import ROOT, RAW, get

KEEP = 400
CANDIDATES = 800
START = dt.date(2024, 1, 2)
FINRA_START = dt.date(2024, 9, 3)
RANK_FROM, RANK_TO = dt.date(2024, 10, 1), dt.date(2024, 10, 31)
YAHOO = RAW / "yahoo_recent"
FINRA = RAW / "finra_recent"
FULL_JAN = RAW / "finra_full_2024_10"
END = dt.date.today()


def weekdays(a, b):
    return [a + dt.timedelta(n) for n in range((b - a).days + 1) if (a + dt.timedelta(n)).weekday() < 5]


def finra_url(d):
    return f"https://cdn.finra.org/equity/regsho/daily/CNMSshvol{d:%Y%m%d}.txt"


def fetch_full(d):
    p = FULL_JAN / f"{d:%Y%m%d}.txt"
    if not p.exists():
        body = get(finra_url(d))
        p.write_bytes(body or b"")
    return p.read_text(errors="replace")


def chart(sym):
    p = YAHOO / f"{sym}.json"
    if not p.exists():
        p1 = int(dt.datetime(START.year, START.month, START.day).timestamp())
        url = (f"https://query1.finance.yahoo.com/v8/finance/chart/{sym}?period1={p1}&period2={int(time.time())}"
               "&interval=1d&events=div%2Csplit")
        body = get(url)
        if body is None:
            return sym, None
        p.write_bytes(body)
    try:
        res = json.loads(p.read_text())["chart"]["result"][0]
        res["timestamp"]
    except (KeyError, TypeError, IndexError):
        return sym, None
    return sym, res


def main():
    for d in (YAHOO, FINRA, FULL_JAN):
        d.mkdir(parents=True, exist_ok=True)
    jan = weekdays(RANK_FROM, RANK_TO)
    with ThreadPoolExecutor(4) as ex:
        texts = list(ex.map(fetch_full, jan))
    vol = collections.defaultdict(list)
    for t in texts:
        for line in t.splitlines():
            p = line.split("|")
            if len(p) >= 5 and p[1].isalpha() and len(p[1]) <= 5 and p[4].replace(".", "").isdigit():
                vol[p[1]].append(float(p[4]))
    cands = [s for s, v in sorted(vol.items(), key=lambda kv: -sum(kv[1]))][:CANDIDATES]
    print(f"oct2024 finra symbols={len(vol)} candidates={len(cands)}", flush=True)
    with ThreadPoolExecutor(4) as ex:
        charts = dict(ex.map(chart, cands))
    missing = [s for s, r in charts.items() if r is None]
    ranked, not_equity, no_jan = [], 0, 0
    for s, r in charts.items():
        if r is None:
            continue
        if r.get("meta", {}).get("instrumentType") != "EQUITY":
            not_equity += 1
            continue
        q = r["indicators"]["quote"][0]
        dv = []
        for i, ts in enumerate(r["timestamp"]):
            d = dt.datetime.utcfromtimestamp(ts).date()
            if RANK_FROM <= d <= RANK_TO and q["close"][i] and q["volume"][i]:
                dv.append((q["close"][i], q["close"][i] * q["volume"][i]))
        if len(dv) < 10:
            no_jan += 1
            continue
        if st.mean(x[0] for x in dv) >= 5:
            ranked.append((st.mean(x[1] for x in dv), s))
    ranked.sort(reverse=True)
    universe = [s for _, s in ranked[:KEEP]]
    (ROOT / "config" / "universe_recent.txt").write_text(" ".join(universe) + "\n")
    print(f"yahoo_missing={len(missing)} not_equity={not_equity} no_rank_window_bars={no_jan} eligible={len(ranked)} kept={len(universe)}", flush=True)
    keep = set(universe)

    def fetch_filtered(d):
        p = FINRA / f"{d:%Y%m%d}.txt"
        if not p.exists():
            body = get(finra_url(d))
            text = ""
            if body is not None:
                text = "\n".join(l for l in body.decode("utf-8", "replace").splitlines() if l.split("|")[1:2] and l.split("|")[1] in keep)
            p.write_text(text)
        return d, p.read_text()

    days = weekdays(FINRA_START, END)
    with ThreadPoolExecutor(8) as ex:
        finra = {}
        for d, t in ex.map(fetch_filtered, days):
            for line in t.splitlines():
                p = line.split("|")
                if len(p) >= 5:
                    finra[(d, p[1])] = (float(p[2]), float(p[4]))
    spy = chart("SPY")[1]
    rows = n_fin = 0
    with (ROOT / "data" / "panel_recent.csv").open("w", newline="") as f:
        w = csv.writer(f)
        w.writerow("date symbol open high low close volume finra_short finra_total".split())
        for sym in universe + ["SPY"]:
            r = spy if sym == "SPY" else charts[sym]
            q = r["indicators"]["quote"][0]
            adj = r["indicators"]["adjclose"][0]["adjclose"]
            splits = [(dt.date.fromtimestamp(s["date"]), s["numerator"] / s["denominator"]) for s in r.get("events", {}).get("splits", {}).values()]
            for i, ts in enumerate(r["timestamp"]):
                d = dt.datetime.utcfromtimestamp(ts).date()
                vals = [q["open"][i], q["high"][i], q["low"][i], q["close"][i], q["volume"][i], adj[i]]
                if any(v is None for v in vals) or vals[3] <= 0:
                    continue
                a = vals[5] / vals[3]
                fs, ft = finra.get((d, sym), (0.0, 0.0))
                factor = 1.0
                for sd, ratio in splits:
                    if d < sd:
                        factor *= ratio
                n_fin += ft > 0
                w.writerow([d.isoformat(), sym, f"{vals[0]*a:.6f}", f"{vals[1]*a:.6f}", f"{vals[2]*a:.6f}", f"{vals[3]*a:.6f}", int(vals[4]), int(fs * factor), int(ft * factor)])
                rows += 1
    print(f"rows={rows} finra_matched={n_fin} symbols={len(universe)+1}", flush=True)


if __name__ == "__main__":
    main()
