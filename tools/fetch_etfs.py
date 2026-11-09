#!/usr/bin/env python3
"""Writes data/etf_panel.csv: dividend-adjusted opens and closes for the trend sleeves."""
import csv, datetime as dt, json, pathlib, time, urllib.request
ROOT = pathlib.Path(__file__).resolve().parent.parent
SYMS = ["SPY", "EFA", "IEF", "GSG", "VNQ"]
n = 0
with (ROOT / "data" / "etf_panel.csv").open("w", newline="") as f:
    w = csv.writer(f)
    w.writerow(["date", "symbol", "open", "adj_close", "raw_close"])
    for s in SYMS:
        url = f"https://query1.finance.yahoo.com/v8/finance/chart/{s}?period1={int(dt.datetime(2001,1,2).timestamp())}&period2={int(time.time())}&interval=1d&events=div%2Csplit"
        req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0 (compatible; research-data-script)"})
        res = json.loads(urllib.request.urlopen(req, timeout=60).read())["chart"]["result"][0]
        q, adj = res["indicators"]["quote"][0], res["indicators"]["adjclose"][0]["adjclose"]
        first = None
        for i, ts in enumerate(res["timestamp"]):
            if None in (q["open"][i], q["close"][i], adj[i]):
                continue
            d = dt.datetime.utcfromtimestamp(ts).date().isoformat()
            first = first or d
            a = adj[i] / q["close"][i]
            w.writerow([d, s, f"{q['open'][i]*a:.6f}", f"{adj[i]:.6f}", f"{q['close'][i]:.6f}"])
            n += 1
        print(s, "first", first, flush=True)
print("rows", n)
