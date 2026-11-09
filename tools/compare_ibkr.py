#!/usr/bin/env python3
"""Compares IBKR daily bars with the Yahoo bars behind the research panel."""
import collections, csv, datetime as dt, json, pathlib, statistics as st

ROOT = pathlib.Path(__file__).resolve().parent.parent
ib = {}
for r in csv.DictReader(open(ROOT / "data/ibkr_daily.csv")):
    d = r["date"]
    ib[(f"{d[:4]}-{d[4:6]}-{d[6:]}", r["symbol"])] = (float(r["open"]), float(r["close"]))
yh = {}
for p in (ROOT / "data/raw/yahoo").glob("*.json"):
    res = json.loads(p.read_text())["chart"]["result"][0]
    q = res["indicators"]["quote"][0]
    for i, ts in enumerate(res["timestamp"]):
        o, c = q["open"][i], q["close"][i]
        if o and c:
            yh[(dt.datetime.utcfromtimestamp(ts).date().isoformat(), p.stem)] = (o, c)
keys = sorted(set(ib) & set(yh))
print(f"ibkr={len(ib)} yahoo={len(yh)} overlap={len(keys)} first={keys[0][0]} last={keys[-1][0]}")
for name, idx in (("open", 0), ("close", 1)):
    diffs = [abs(ib[k][idx] / yh[k][idx] - 1) * 1e4 for k in keys]
    diffs.sort()
    n = len(diffs)
    print(f"{name}: median={diffs[n//2]:.2f}bp p90={diffs[int(n*.9)]:.2f}bp p99={diffs[int(n*.99)]:.2f}bp max={diffs[-1]:.0f}bp  >5bp={sum(x>5 for x in diffs)/n:.2%}  >25bp={sum(x>25 for x in diffs)/n:.2%}")
by = collections.defaultdict(list)
for k in keys:
    by[k[0][:4]].append(abs(ib[k][0] / yh[k][0] - 1) * 1e4)
print("open median bp by year:", {y: round(st.median(v), 2) for y, v in sorted(by.items())})
sign = [(ib[k][0] / yh[k][0] - 1) * 1e4 for k in keys]
print(f"open signed mean bp (IBKR minus Yahoo): {st.mean(sign):+.3f}")
worst = sorted(keys, key=lambda k: -abs(ib[k][0] / yh[k][0] - 1))[:8]
for k in worst:
    print("  ", k, "ibkr", ib[k][0], "yahoo", round(yh[k][0], 4))
