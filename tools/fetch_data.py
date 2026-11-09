#!/usr/bin/env python3
"""Build data/panel.csv: split/dividend-adjusted daily bars (Yahoo) joined with
FINRA daily off-exchange and short volume. Raw responses are cached in data/raw."""
import csv, datetime as dt, json, pathlib, sys, time, urllib.error, urllib.request
from concurrent.futures import ThreadPoolExecutor

ROOT = pathlib.Path(__file__).resolve().parent.parent
RAW = ROOT / "data" / "raw"
START = dt.date(2018, 6, 1)
FINRA_START = dt.date(2019, 1, 1)
END = dt.date.today()
UA = {"User-Agent": "Mozilla/5.0 (compatible; research-data-script)"}
# --live refetches bars into a separate panel, leaving the frozen research panel untouched.
LIVE = "--live" in sys.argv
YAHOO_DIR = "yahoo_live" if LIVE else "yahoo"
OUTPUT = "panel_live.csv" if LIVE else "panel.csv"
SYMBOLS = (ROOT / "config" / "universe.txt").read_text().split()
BENCH = ["SPY"]


def get(url, tries=4):
    for attempt in range(tries):
        try:
            with urllib.request.urlopen(urllib.request.Request(url, headers=UA), timeout=30) as r:
                return r.read()
        except urllib.error.HTTPError as e:
            if e.code in (403, 404):
                return None
            time.sleep(1 + attempt * 2)
        except Exception:
            time.sleep(1 + attempt * 2)
    raise RuntimeError(f"failed: {url}")


def fetch_chart(sym):
    path = RAW / YAHOO_DIR / f"{sym}.json"
    if not path.exists():
        p1 = int(dt.datetime(START.year, START.month, START.day).timestamp())
        p2 = int(time.time())
        url = (f"https://query1.finance.yahoo.com/v8/finance/chart/{sym}"
               f"?period1={p1}&period2={p2}&interval=1d&events=div%2Csplit")
        body = get(url)
        if body is None:
            print(f"dropped {sym}: no chart returned", flush=True)
            return None
        path.write_bytes(body)
    res = json.loads(path.read_text())["chart"]["result"][0]
    q = res["indicators"]["quote"][0]
    adj = res["indicators"]["adjclose"][0]["adjclose"]
    splits = [(dt.date.fromtimestamp(s["date"]), s["numerator"] / s["denominator"])
              for s in res.get("events", {}).get("splits", {}).values()]
    rows = {}
    for i, ts in enumerate(res["timestamp"]):
        d = dt.datetime.utcfromtimestamp(ts).date()
        vals = [q["open"][i], q["high"][i], q["low"][i], q["close"][i], q["volume"][i], adj[i]]
        if any(v is None for v in vals) or vals[3] <= 0:
            continue
        f = vals[5] / vals[3]
        rows[d] = (vals[0] * f, vals[1] * f, vals[2] * f, vals[3] * f, vals[4])
    return sym, rows, splits


def fetch_finra(day):
    path = RAW / "finra" / f"{day:%Y%m%d}.txt"
    recent = (END - day).days <= 5
    if path.exists() and not (LIVE and recent and path.stat().st_size == 0):
        return path.read_text()
    body = get(f"https://cdn.finra.org/equity/regsho/daily/CNMSshvol{day:%Y%m%d}.txt")
    keep = set(SYMBOLS)
    text = ""
    if body is not None:
        lines = body.decode("utf-8", "replace").splitlines()
        text = "\n".join(l for l in lines if l.split("|")[1:2] and l.split("|")[1] in keep)
    path.write_text(text)
    return text


def main():
    if LIVE:
        for old in (RAW / YAHOO_DIR).glob("*.json"):
            old.unlink()
    (RAW / YAHOO_DIR).mkdir(parents=True, exist_ok=True)
    (RAW / "finra").mkdir(parents=True, exist_ok=True)
    with ThreadPoolExecutor(4) as ex:
        charts = [c for c in ex.map(fetch_chart, SYMBOLS + BENCH) if c is not None]
    days = [FINRA_START + dt.timedelta(n) for n in range((END - FINRA_START).days + 1)]
    days = [d for d in days if d.weekday() < 5]
    with ThreadPoolExecutor(8) as ex:
        texts = list(ex.map(fetch_finra, days))
    finra = {}
    for d, text in zip(days, texts):
        for line in text.splitlines():
            p = line.split("|")
            if len(p) >= 5:
                finra[(d, p[1])] = (float(p[2]), float(p[4]))
    out = ROOT / "data" / OUTPUT
    n = hits = 0
    with out.open("w", newline="") as f:
        w = csv.writer(f)
        w.writerow("date symbol open high low close volume finra_short finra_total".split())
        for sym, rows, splits in charts:
            for d in sorted(rows):
                o, h, l, c, v = rows[d]
                fs, ft = finra.get((d, sym), (0.0, 0.0))
                factor = 1.0
                for sd, ratio in splits:
                    if d < sd:
                        factor *= ratio
                hits += ft > 0
                w.writerow([d.isoformat(), sym, f"{o:.6f}", f"{h:.6f}", f"{l:.6f}", f"{c:.6f}",
                            int(v), int(fs * factor), int(ft * factor)])
                n += 1
    print(f"rows={n} finra_matched={hits} symbols={len(charts)} finra_days_requested={len(days)}")


if __name__ == "__main__":
    main()
