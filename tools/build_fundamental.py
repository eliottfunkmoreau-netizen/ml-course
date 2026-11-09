#!/usr/bin/env python3
"""Downloads first-reported annual fundamentals (SEC data API) and daily prices (Yahoo).
Only companies listed today can be priced, which leaves survivorship bias: see README."""
import csv, datetime as dt, gzip, json, pathlib, threading, time, urllib.error, urllib.request
from concurrent.futures import ThreadPoolExecutor

ROOT = pathlib.Path(__file__).resolve().parent.parent
RAW = ROOT / "data" / "raw" / "edgar"
PRICE = ROOT / "data" / "raw" / "yahoo_fund"
UA = "quant-research-script/1.0 (non-commercial research)"
FLOWS = ["GrossProfit", "Revenues", "RevenueFromContractWithCustomerExcludingAssessedTax", "SalesRevenueNet", "CostOfRevenue", "CostOfGoodsAndServicesSold", "CostOfGoodsSold"]
STOCKS = ["Assets", "StockholdersEquity"]
_lock, _next = threading.Lock(), [0.0]


def sec(url, path):
    if path.exists():
        return json.loads(path.read_text())
    with _lock:
        wait = _next[0] - time.time()
        _next[0] = max(time.time(), _next[0]) + 0.13
    if wait > 0:
        time.sleep(wait)
    for attempt in range(5):
        try:
            r = urllib.request.urlopen(urllib.request.Request(url, headers={"User-Agent": UA, "Accept-Encoding": "gzip"}), timeout=60)
            b = r.read()
            text = (gzip.decompress(b) if r.headers.get("Content-Encoding") == "gzip" else b).decode()
            path.write_text(text)
            return json.loads(text)
        except urllib.error.HTTPError as e:
            if e.code == 404:
                return None
            time.sleep(2 + attempt * 3)
        except Exception:
            time.sleep(2 + attempt * 3)
    return None


def compact(path):
    return path


def meta(cik):
    d = sec(f"https://data.sec.gov/submissions/CIK{cik:010d}.json", RAW / "sub" / f"{cik}.json")
    if not d or not d.get("tickers") or not d.get("exchanges"):
        return None
    sic = d.get("sic") or ""
    if d.get("entityType") != "operating" or d["exchanges"][0] not in ("NYSE", "Nasdaq") or not sic or 6000 <= int(sic) <= 6999:
        return None
    return cik, d["tickers"][0], sic


def facts(m):
    cik, ticker, sic = m
    d = sec(f"https://data.sec.gov/api/xbrl/companyfacts/CIK{cik:010d}.json", RAW / "facts" / f"{cik}.json")
    if not d:
        return []
    g = d.get("facts", {}).get("us-gaap", {})
    best = {}
    for tag in FLOWS + STOCKS:
        for x in g.get(tag, {}).get("units", {}).get("USD", []):
            if x.get("form") != "10-K":
                continue
            if tag in FLOWS:
                if "start" not in x:
                    continue
                n = (dt.date.fromisoformat(x["end"]) - dt.date.fromisoformat(x["start"])).days
                if not 350 <= n <= 380:
                    continue
            k = (tag, x["end"])
            if k not in best or x["filed"] < best[k]["filed"]:
                best[k] = x
    rows = [[cik, ticker, sic, t, e, x["val"], x["filed"]] for (t, e), x in best.items()]
    seen = set()
    for x in d.get("facts", {}).get("dei", {}).get("EntityCommonStockSharesOutstanding", {}).get("units", {}).get("shares", []):
        k = (x["end"], x["filed"])
        if k not in seen and x.get("val"):
            seen.add(k)
            rows.append([cik, ticker, sic, "Shares", x["end"], x["val"], x["filed"]])
    return rows


def price(sym):
    p = PRICE / f"{sym}.json"
    if not p.exists():
        p1, p2 = int(dt.datetime(2011, 1, 3).timestamp()), int(time.time())
        url = f"https://query1.finance.yahoo.com/v8/finance/chart/{sym}?period1={p1}&period2={p2}&interval=1d&events=div%2Csplit"
        for attempt in range(4):
            try:
                body = urllib.request.urlopen(urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0 (compatible; research-data-script)"}), timeout=30).read()
                p.write_bytes(body)
                break
            except urllib.error.HTTPError as e:
                if e.code in (403, 404):
                    return sym, None
                time.sleep(1 + attempt * 2)
            except Exception:
                time.sleep(1 + attempt * 2)
        else:
            return sym, None
    try:
        res = json.loads(p.read_text())["chart"]["result"][0]
        res["timestamp"]
        return sym, res
    except Exception:
        return sym, None


def main():
    for d in (RAW / "sub", RAW / "facts", PRICE):
        d.mkdir(parents=True, exist_ok=True)
    frame = sec("https://data.sec.gov/api/xbrl/frames/us-gaap/Assets/USD/CY2024Q4I.json", RAW / "frame.json")
    ciks = sorted({x["cik"] for x in frame["data"]})
    print("frame companies", len(ciks), flush=True)
    with ThreadPoolExecutor(8) as ex:
        ms = [m for m in ex.map(meta, ciks) if m]
    print("listed non-financial operating companies", len(ms), flush=True)
    with ThreadPoolExecutor(8) as ex:
        all_rows = list(ex.map(facts, ms))
    usable = [(m, r) for m, r in zip(ms, all_rows) if any(x[3] == "Assets" for x in r) and any(x[3] in FLOWS for x in r) and any(x[3] == "Shares" for x in r)]
    print("with 10-K assets, income and shares", len(usable), flush=True)
    with (ROOT / "data" / "fund_facts.csv").open("w", newline="") as f:
        w = csv.writer(f)
        w.writerow("cik ticker sic tag end val filed".split())
        for _, rows in usable:
            w.writerows(rows)
    syms = sorted({m[1] for m, _ in usable})
    with ThreadPoolExecutor(4) as ex:
        charts = dict(ex.map(price, syms + ["SPY"]))
    missing = [s for s, r in charts.items() if r is None]
    print("priced", len(charts) - len(missing), "missing", len(missing), flush=True)
    n = 0
    with (ROOT / "data" / "fund_prices.csv").open("w", newline="") as f:
        w = csv.writer(f)
        w.writerow("date symbol open adj_close raw_close volume".split())
        for s, r in charts.items():
            if r is None:
                continue
            q = r["indicators"]["quote"][0]
            adj = r["indicators"]["adjclose"][0]["adjclose"]
            splits = [(dt.date.fromtimestamp(x["date"]), x["numerator"] / x["denominator"]) for x in r.get("events", {}).get("splits", {}).values()]
            for i, ts in enumerate(r["timestamp"]):
                vals = [q["open"][i], q["close"][i], q["volume"][i], adj[i]]
                if any(v is None for v in vals) or vals[1] <= 0:
                    continue
                d = dt.datetime.utcfromtimestamp(ts).date()
                factor = 1.0
                for sd, ratio in splits:
                    if d < sd:
                        factor *= ratio
                a = vals[3] / vals[1]
                w.writerow([d.isoformat(), s, f"{vals[0]*a:.6f}", f"{vals[3]:.6f}", f"{vals[1]*factor:.6f}", int(vals[2] / factor)])
                n += 1
    print("price rows", n, flush=True)


if __name__ == "__main__":
    main()
