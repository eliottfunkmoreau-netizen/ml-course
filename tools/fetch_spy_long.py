#!/usr/bin/env python3
"""Writes data/spy_long.csv with SPY raw and dividend-adjusted closes since inception."""
import csv, datetime as dt, json, pathlib, time, urllib.request
ROOT = pathlib.Path(__file__).resolve().parent.parent
url = f"https://query1.finance.yahoo.com/v8/finance/chart/SPY?period1={int(dt.datetime(1993,1,20).timestamp())}&period2={int(time.time())}&interval=1d&events=div%2Csplit"
req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0 (compatible; research-data-script)"})
res = json.loads(urllib.request.urlopen(req, timeout=60).read())["chart"]["result"][0]
q = res["indicators"]["quote"][0]
adj = res["indicators"]["adjclose"][0]["adjclose"]
n = 0
with (ROOT / "data" / "spy_long.csv").open("w", newline="") as f:
    w = csv.writer(f)
    w.writerow(["date", "close", "adj_close"])
    for i, ts in enumerate(res["timestamp"]):
        if q["close"][i] is None or adj[i] is None:
            continue
        w.writerow([dt.datetime.utcfromtimestamp(ts).date().isoformat(), f"{q['close'][i]:.6f}", f"{adj[i]:.6f}"])
        n += 1
print("rows", n)
