# LLM quant research pipeline

A C++20 implementation of this flow, with the evaluation design taken from
*Hard to Fool* (arXiv:2611.06607) and *Quant-Arena* (arXiv:2610.30113):

```
alternative data -> LLM proposes hypotheses -> backtest + simulation -> reject / pass gates
                                                                          -> portfolio -> execution
```

The model proposes and trusted code decides. Proposals are JSON over a fixed vocabulary, so no
model-written code ever runs.

## Build and test

```bash
make -j4 && make test
```

Needs only Apple clang and make.

## Research lifecycle

```bash
python3 tools/fetch_data.py
./build/quant freeze runs/NAME config/contract.json data/panel.csv proposals
./build/quant discover runs/NAME
./build/quant validate runs/NAME
./build/quant holdout runs/NAME
./build/quant verify runs/NAME
```

| Control | Implementation |
| --- | --- |
| Declarative proposals | `include/quant/hypothesis.hpp` rejects unknown fields, features and out-of-range values |
| Admission | Near-duplicates and over-quota families are refused before spending a trial |
| Stage separation | `Panel::up_to` physically drops rows after the stage end |
| One-use holdout | A durable exclusive receipt is written before holdout data is read |
| Gates | `include/quant/gates.hpp`, all must pass; ranking cannot admit a candidate |
| Audit trail | `runs/NAME/ledger.tsv` is hash-chained and verified on every command |
| Cost model | IBKR tiered commission at the account's ticket size, plus 3 bps slippage per side |

Gates: positive net P&L, positive excess over the equal-weight universe, positive weekly-cluster
bootstrap bound on excess, positive drift-neutral bootstrap bound, positive excess at doubled
costs, no negative year, opposite side does not pass, and trade and week floors.

## Current results

0 of 19 pre-registered strategies have passed, so nothing is promoted and validation and holdout
are still sealed.

| Run | Test | Sample | Strategies | Passed | Main reading |
| --- | --- | --- | --- | --- | --- |
| `week-2026-11-09` | Cross-sectional rules, 97 large caps | Jul 2019 to Dec 2022 | 12 | 0 | Also 0 of 12 at zero cost |
| `campaign-2` | Crash-day losers | same | 2 | 0 | |
| `campaign-3-recent` | Off-exchange share, 397 stocks | Nov 2024 to Dec 2025 | 2 | 0 | Excess −8 and −12 bp/day |
| `h1-turn-of-month` | SPY turn of month | 2006 to 2026 | 1 | 0 | Window vs other days t = +0.23 |
| `h2-value-profitability-r2` | Top 10 by value and profitability, yearly | Jun 2012 to Nov 2026 | 1 | 0 | +246% vs +550% for equal weight, despite survivorship bias in its favour |
| `h5-trend` | Five-ETF 10-month trend | May 2007 to Nov 2026 | 1 | 0 | +129% vs +178%; max drawdown −15% vs −49% |

`runs/h2-value-profitability` is void: its receipt was claimed before a CSV parsing error stopped
the run, and no simulation took place (see `VOID.txt` there).

The `target` and `forward` commands refuse to act without a promoted strategy. `--shadow <id>` runs
a labelled diagnostic of a rejected candidate.

## Live OTC dashboard

```bash
make build/quant_dash && ./build/quant_dash
```

Then open http://127.0.0.1:8877. It needs IB Gateway on port 4001 and an OTC Markets Level 1
subscription. `quant_dash` is one C++ process: it reads quotes and trades over the read-only wire
protocol in `include/quant/ib_wire.hpp`, computes the analytics in `include/quant/otc_book.hpp`,
and pushes them to `web/otc.html` over server-sent events. It listens on loopback only and cannot
send orders. The watchlist is `config/otc_watchlist.txt`, editable from the page.

Options: `--port`, `--gateway-port`, `--client-id` (default 19), `--ticket` (dollars, default 500),
`--max-lines` (default 80), `--flush-ms` (default 25), `--mode realtime|delayed`.

Level 1 only: there is no depth of book. The LQ grade ranks how cheaply a name trades, not whether
to buy it. Session times ignore exchange holidays.

## Known limitations

- **The research binary has no live link.** `quant_feed` and `quant_dash` read live IBKR data, but
  `quant` still backtests on files. History comes from Yahoo and FINRA; sampled SPY closes matched
  IBKR bars.
- **No order routing.** Execution is simulated.
- **Proposals are static files.** There is no automated LLM call yet. The 12 in `proposals/` were
  written by Claude before any result was seen.
- **Survivorship bias.** The universe is 97 stocks that are large today, which flatters results.
  MMC was dropped because Yahoo returned no data. FINRA rows are missing for about 1% of
  stock-days, including before ticker changes.
- **Latency.** `quant bench` measures only the in-process path. It excludes the network, the
  gateway and the exchange. Daily strategies gain nothing from it.
