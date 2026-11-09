# LLM text signals (filings, earnings calls, news): tradability under look-ahead-safe evaluation, and retail implementation

Scope note. Seven papers were read in full-text PDF, extracted locally with pypdf: Chen and Pu; Li et al. (DeepFund); Lopez-Lira and Tang; Choi et al.; Barton et al.; Gencay; Cohen, Malloy and Nguyen (Lazy Prices). Lopez-Lira and Tang (142 pp.) and Lazy Prices (90 pp.) were read by targeted search and section reads, not line by line; the other five were read in full, except DeepFund, whose results, limitation and setup sections were read. No PDF in this set was abstract-only. Everything labelled "my arithmetic" is my own calculation from the cited inputs, not a paper result.

Account assumptions used throughout (from the brief): USD 2,460 account, USD 0.35 minimum commission per order, long-biased, few positions, days-to-months holding. Assistant training data runs through about June 2026.

## For each paper: citation, test, sample, net-of-cost result, turnover, out-of-sample status, cost issues at USD 2,500

### Takeaway
No paper in this set clears the evaluation bar. The best live evidence (Chen and Pu) covers about nine months, reports a text-table mismatch in its own headline numbers and counts only spreads as costs; the most rigorous evaluation (Gencay) rejects every LLM-discovered strategy, though on price features only.

### Cited Findings

**1. Chen, Z. and Pu, D. (Jan 2026). "Autonomous Market Intelligence: Agentic AI Nowcasting Predicts Stock Returns." arXiv:2601.11958v1. Full PDF read.**
- Tested: a web-interface LLM with web search scores every Russell 1000 stock daily from -5 to +5 at horizons from 1 day to 1 year. The model is described only as "a leading US AI model", and the latest version is always used, so the model changes during the sample — [Chen and Pu](https://arxiv.org/abs/2601.11958)
- Sample: April 2025 to January 2026, nearly 156,000 stock-day observations, 158 daily portfolio observations in the main tables — [Chen and Pu](https://arxiv.org/abs/2601.11958)
- Timing: prompts entered after the close of t-1 and finished before the open of t; returns are open-to-open from t to t+1, entered at the opening auction — [Chen and Pu](https://arxiv.org/abs/2601.11958)
- Headline as stated in the text: Top-20 value-weighted daily FF6 alpha 0.184% per day (t=2.46), Sharpe 2.43, about 50% cumulative return against 26% for the Russell 1000 — [Chen and Pu](https://arxiv.org/abs/2601.11958)
- Internal inconsistency: Table 4 column 5 reports FF6 alpha 0.334% (t=2.96), and Table 5 reports Top-20 daily Sharpe 3.23 and long-short Sharpe 2.23, whereas the text reports 0.184%, 2.43 and 1.46 for the same objects. Weekly and quarterly figures also differ between text and table (weekly 0.17% t=2.32 in text, 0.238% t=2.72 in Table 5) — [Chen and Pu](https://arxiv.org/abs/2601.11958)
- Net of cost: no net return series is reported. The paper argues average portfolio spreads of 1.63 bps (daily signal) are under 10% of gross alpha. Commissions, impact and full round-trip cost are not modelled — [Chen and Pu](https://arxiv.org/abs/2601.11958)
- Turnover: 57.4% per day (daily signal), 72.3% per day (weekly signal); the longer-horizon signals turn over more — [Chen and Pu](https://arxiv.org/abs/2601.11958)
- Out-of-sample: live by construction at each prediction date. Consistency check: a single query correlates only 0.629 (Spearman) with the mean of about 350 repeated queries — [Chen and Pu](https://arxiv.org/abs/2601.11958)
- Post-publication: the GitHub README is dated January 11, 2026 and shows no post-January-2026 results on the page; `data.json` exists but was not inspected — [repository](https://github.com/mapledust0/AI-Stock-Nowcasting)
- Liquidity: median market cap USD 15.2bn, median spread 3.41 bps. Top-20 appearance frequency is led by Broadcom (0.630), NVIDIA (0.607), Microsoft (0.543) — [Chen and Pu](https://arxiv.org/abs/2601.11958)

**2. Li, C., Shi, Y., Wang, C., Duan, Q., Ruan, R., Huang, W., Long, H., Huang, L., Tang, N. and Luo, Y. "Time Travel is Cheating: Going Live with DeepFund for Real-Time Fund Investment Benchmarking." arXiv:2505.11065v2. PDF read for setup, results, limitations.**
- Tested: nine LLMs running a multi-agent live pipeline on market data published after each model's training cutoff — [DeepFund](https://arxiv.org/abs/2505.11065)
- Sample: March 17 to April 17, 2025, 24 trading days, 1,080 decisions in total — [DeepFund](https://arxiv.org/abs/2505.11065)
- Result: eight of nine LLMs had negative cumulative return; only Grok 3 mini Beta was positive (+1.1% against -3.09% buy-and-hold on its row). Claude 3.7 Sonnet -3.7%, DeepSeek-V3 -5.7%, GPT-4.1 -5.9%, Doubao-1.5-pro -8.1% (Table 3) — [DeepFund](https://arxiv.org/abs/2505.11065)
- Net of cost: not net. The paper states the implementation does not account for transaction fees or market trading restrictions, and that the period was short and volatile — [DeepFund](https://arxiv.org/abs/2505.11065)
- Turnover: not reported — [DeepFund](https://arxiv.org/abs/2505.11065)

**3. Lopez-Lira, A. and Tang, Y. "Can ChatGPT Forecast Stock Price Movements? Return Predictability and Large Language Models." arXiv:2304.07619v6 (Oct 28, 2025). PDF read by search and section reads.**
- Tested: GPT-4 and other LLMs score headlines for 4,123 US common stocks, October 2021 to May 2024. The paper says ChatGPT training data stops in September 2021, so the sample is post-cutoff — [Lopez-Lira and Tang](https://arxiv.org/abs/2304.07619)
- Gross: long-short overnight drift strategy earns 34 bps per day before costs, Sharpe 2.97 for overnight news and 2.63 for intraday news — [Lopez-Lira and Tang](https://arxiv.org/abs/2304.07619)
- Long leg against short leg (overnight): long earns 8 bps per day (Sharpe 0.78); short earns 26 bps per day (Sharpe 2.01) — [Lopez-Lira and Tang](https://arxiv.org/abs/2304.07619)
- Cost sensitivity: at 5 bps round-trip the cumulative return is over 300%; at 10 bps above 100%; at 20 bps round-trip the strategy is unprofitable. The authors state exploitation is feasible only for participants with sufficiently low costs, such as market makers — [Lopez-Lira and Tang](https://arxiv.org/abs/2304.07619)
- Turnover: about 190% per day at 100% daily rebalancing, about 46% at 25% rebalancing. Sharpe at 10 bps round-trip is 1.29 (100%) and 1.34 (25%) — [Lopez-Lira and Tang](https://arxiv.org/abs/2304.07619)
- Decay: annualized Sharpe falls from 6.54 in 2021Q4 to 3.68 in 2022, 2.33 in 2023 and 1.22 in January to May 2024, which the authors present as suggestive evidence of LLM adoption, while noting other factors could contribute — [Lopez-Lira and Tang](https://arxiv.org/abs/2304.07619)

**4. Choi, C., Kim, Y., Yu, Y., Cha, Y., Golkhou, V.Z., Halperin, I., Papaioannou, G., Kim, M., Wang, Z., Kwon, J., Kim, M., Lopez-Lira, A. and Lee, Y. "From Text to Alpha: Can LLMs Track Evolving Signals in Corporate Disclosures?" arXiv:2510.03195v5 (Mar 15, 2026). Full PDF read.**
- Tested: "moving targets" on S&P 100 earnings-call transcripts. Gemini-2.5-Pro extracts performance metrics; text-embedding-3-large measures whether year-ago metrics persist. Score: MT = 1 - mean(S), where S is a thresholded max cosine similarity (0 at or below 0.4, 1 at or above 0.6, linear between) — [Choi et al.](https://arxiv.org/abs/2510.03195)
- Sample: January 2010 to December 2024, 5,615 firm-quarters. The paper reports 64 quarters; that span contains 60 calendar quarters — [Choi et al.](https://arxiv.org/abs/2510.03195)
- Result (gross, equal-weight quintiles, Table 3): Q5 minus Q1 excess -0.41% per month (t=-2.08), 3-factor alpha -0.48% (t=-2.40), 5-factor alpha -0.52% (t=-2.55) — [Choi et al.](https://arxiv.org/abs/2510.03195)
- Long-side evidence: Q1 (low shifting) 5-factor alpha 0.55% per month (t=3.71), Q1 excess 1.48%; Q5 (high shifting) 5-factor alpha 0.13% (t=0.87), Q5 excess 1.17% — [Choi et al.](https://arxiv.org/abs/2510.03195)
- Internal inconsistency: the Fama-MacBeth coefficient is -0.0370 with t=-0.95 in Table 4, while the conclusion says the regressions confirm significance at the 10% level — [Choi et al.](https://arxiv.org/abs/2510.03195)
- Net of cost and turnover: neither reported; only gross alphas. Thresholds were selected from a sweep using predictive performance and interpretability — [Choi et al.](https://arxiv.org/abs/2510.03195)
- Look-ahead: no post-cutoff or point-in-time test is reported, and the authors note the S&P 100 restriction as a limitation — [Choi et al.](https://arxiv.org/abs/2510.03195)

**5. Barton, T.J., Constantakis, C., Hauseman, P., Mous, A., Hoffman, A., Bergeron, B. and Goodreau, H. (Sept 2026). "What LLM Trading Agents Actually Do in Production: A Six-Month, Population-Scale Record from Two Fleets." arXiv:2609.05663v1. Full PDF read.**
- Tested: DX Terminal Pro (3,505 real-ETH memecoin vaults, 21 days, Feb to Mar 2026) and DXAP (Hyperliquid perpetuals, Jun 8 to Aug 15, 2026, mostly paper at USD 10,000 with 5,035 real-money fills). Crypto, not equities — [Barton et al.](https://arxiv.org/abs/2609.05663)
- Result: DXAP cumulative realized P&L -USD 217K at a common 5.5 bps fee, against -USD 148K at zero fee; so fees are not the explanation. Roundtrip win rate 41% against 50% for matched retail; 15% of agents net positive against 53% — [Barton et al.](https://arxiv.org/abs/2609.05663)
- Nulls: a 77K-candidate signal ledger sorts at chance (leave-one-out win 0.50); the market-research sub-agent is null (n=120); no group across 8 postures, 6 templates and 9 cohorts has a day-clustered lower bound above zero — [Barton et al.](https://arxiv.org/abs/2609.05663)
- Frictions understated: paper fills have zero slippage and zero funding — [Barton et al.](https://arxiv.org/abs/2609.05663)
- Model replay: three frontier models span 263.27 to 264.37 bps regret over 416 replayed scenarios, Holm-adjusted minimum p=0.46; replay says nothing about live P&L — [Barton et al.](https://arxiv.org/abs/2609.05663)
- Mechanics: a fixed 2%/4% stop/target bracket added +39.0 bps per position [+21.3, +56.5] but none of 16 exit policies was profitable outright (best -9.5 [-25.1, +8.2]) — [Barton et al.](https://arxiv.org/abs/2609.05663)
- Authors are the operators of these systems (DX Research Group); the paper reports three of its own retracted results — [Barton et al.](https://arxiv.org/abs/2609.05663)

**6. Gencay, E. (Aug 27, 2026). "What survives honest evaluation? Leakage-safe, search-aware assessment of LLM-driven trading strategy discovery." arXiv:2608.27734v1. Full PDF read.**
- Tested: LLM agents (gpt-4.1, claude-sonnet-5) propose strategies through a validated tool layer; every trial is recorded and used to deflate results. 453 US large caps, tradeable set re-selected point-in-time as top 200 by trailing 63-day dollar volume, design 2017 to 2021, evaluation 2022 to 2025; also 39 ETFs, evaluation 2017 to 2025 — [Gencay](https://arxiv.org/abs/2608.27734)
- Costs: 1 bp commission, 2 bp spread, square-root impact, 50 bp annual borrow, stressed at x0.5 and x2 — [Gencay](https://arxiv.org/abs/2608.27734)
- Result (net): best gpt-4.1 discovery, design Sharpe 1.69, evaluation Sharpe 0.18, +4.7%, CI [-0.80, +1.14], DSR 0.86 after 102 trials, against equal-weight buy-and-hold +40.8% and SPY +51.5% — [Gencay](https://arxiv.org/abs/2608.27734)
- Cross-model and repeats: claude-sonnet-5 best design Sharpe 0.44, evaluation -0.33 (-29%), DSR 0.18. Five gpt-4.1 repeats: evaluation Sharpe 0.59 to 0.70, DSR 0.15 to 0.29, 0 of 5 survive. ETF universe agent evaluation Sharpe -0.23 (-21%) — [Gencay](https://arxiv.org/abs/2608.27734)
- Calibration: a random-weights null earned +23% over nine evaluation years; a planted look-ahead oracle (Sharpe 35) passes deflated Sharpe completely, so statistical deflation does not catch leakage — [Gencay](https://arxiv.org/abs/2608.27734)
- Scope limit: news-sentiment and point-in-time fundamental features exist in the system but were excluded from the headline discovery runs. The equity list is a fixed current list, so survivorship is present and flatters active strategies. Turnover is not reported — [Gencay](https://arxiv.org/abs/2608.27734)

**7. Cohen, L., Malloy, C. and Nguyen, Q. "Lazy Prices." NBER Working Paper 25084 (Sept 2018, rev. March 2019); published Journal of Finance 75(3), 1371-1415 (2020). PDF read by search and section reads. Not an LLM paper: included as the filings-only, mechanically computable baseline.**
- Tested: year-over-year textual similarity of 10-K and 10-Q filings using four measures (cosine, Jaccard, minimum edit, simple), universe of US firms, 1995 to 2014. Stocks enter the month after release and are held three months, rebalanced monthly — [Lazy Prices](https://www.nber.org/papers/w25084)
- Result (gross): value-weighted long non-changers/short changers earns up to 58 bps per month (t=3.59); returns accrue out to 18 months with no reversal, and there is statistically zero announcement-day return in the full sample — [Lazy Prices](https://www.nber.org/papers/w25084)
- Long leg (Q5, value-weight, 5-factor, Table II): cosine 0.23% per month (t=2.23), Jaccard 0.23% (t=2.11), simple 0.15% (not significant). Equal-weight Q5 5-factor: 0.21% to 0.30% per month — [Lazy Prices](https://www.nber.org/papers/w25084)
- Section result: changes in Risk Factors (Item 1A) yield up to 188 bps per month (t=2.76) in Table VII Panel A; MD&A 11 to 22 bps. Authors call section results suggestive but not definitive — [Lazy Prices](https://www.nber.org/papers/w25084)
- Cost evidence: no net-of-cost return is reported. The authors argue costs are not an explanation because signal is slow, portfolio turnover modest around reporting dates, and value-weighted returns are not concentrated in small firms. Average market value: Q1 USD 3.5bn, Q5 USD 2.5bn; Q5 shorting fees 73.5 bps — [Lazy Prices](https://www.nber.org/papers/w25084)

### Inferences
- **Cost at USD 2,460 (my arithmetic):** a round trip costs USD 0.70 at the minimum commission, which is 8.5 bps of position size at 3 positions (USD 820), 14.2 bps at 5 (USD 492), 28.5 bps at 10 (USD 246) and 56.9 bps at 20 (USD 123). That is before spread.
- **Chen and Pu daily strategy at USD 2,460 (my arithmetic):** Top-20 with 57.4% daily turnover implies about 11.5 names in and 11.5 out, about 23 orders, about USD 8.04 per day, or 32.7 bps of the account per day. This exceeds the 18.4 bps text alpha and is about the size of the 33.4 bps table alpha. Annualized it is about USD 2,025, 82% of the account. Not tradable at this size.
- **Possible regression artifact in Chen and Pu:** portfolio returns are open-to-open, and the paper does not say the factors were re-timed to match, while standard Fama-French daily factors are close-to-close. A portfolio dominated by Broadcom, NVIDIA and Microsoft carrying market beta of 0.35 is consistent with non-synchronous regressors understating beta and inflating alpha. This is my reading; the paper does not test it.
- **Lopez-Lira and Tang at USD 2,460 (my arithmetic):** the 8 bps per day long leg is below the 14.2 bps round-trip commission at 5 positions. The strategy's profit also comes mostly from the short leg, which is excluded here. The 20 bps round-trip failure point is reachable from commission plus spread alone.
- **Lazy Prices long-only at USD 2,460 (my arithmetic):** Q5 value-weight 5-factor alpha of 0.23% per month is about 2.8% per year gross, about USD 68 on the account. Five positions rotated four times a year cost USD 14 per year (0.57%), USD 28 at doubled costs. Commission is affordable, but the signal is thin relative to single-name noise and comes from 1995 to 2014 with no post-publication evidence in what I read.
- **Choi at the long side (my arithmetic):** Q1 excess of 1.48% minus the equal-weighted average of the five quintiles (1.312%) is about 0.17% per month, about 2% per year gross. The opposite-side test fails to separate cleanly, since Q5's raw excess return is still 1.17% per month.
- Gencay is a strong null for price-only LLM strategy discovery, but it does not test LLM text signals, since text features were excluded from the headline runs. It should not be cited as evidence against text signals specifically.
- Barton is relevant as live-logging evidence and methodology, but it is short-horizon crypto with paper fills and does not transfer directly to equities.

### Gaps
- No paper in the set reports a net-of-cost, weekly-cluster-bootstrapped, year-by-year result that can be mapped directly to the evaluation bar. Chen and Pu's sample is under one year, so "no negative year" is undefined.
- I did not inspect `data.json` in the Chen and Pu repository, so post-January-2026 live performance is unknown.
- DeepFund's main ticker list was not located in the sections read; 1,080 decisions over 24 days and 9 models implies 5 per model-day.
- Lazy Prices' "monthly turnover" in Table III (0.066 to 0.087) appears to be stock-level share turnover rather than portfolio turnover; I could not confirm this from the table caption text extracted.

## Which LLM text signals have results dated after the model's training cutoff, or use point-in-time or live logging?

### Takeaway
Chen and Pu, DeepFund and Barton are live-logged; Lopez-Lira and Tang is post-cutoff but scored retrospectively; Choi is contaminated in principle; Gencay is leakage-safe on features but its LLM proposers are not memory-free. Only Chen and Pu shows a positive live equity result, and it is short and internally inconsistent.

### Cited Findings
| Paper | Post-cutoff status | Logging | Text input | Positive after evaluation? |
|---|---|---|---|---|
| Chen and Pu | Live each day, model version changes | Prompts collected between close and open; logged at the decision timestamp | Web search chosen by agent | Gross positive, 9 months |
| DeepFund | Live, post-cutoff data | Live pipeline, Supabase store | Multi-source incl. company news, insider, policy | 8 of 9 negative, 24 days, no fees |
| Lopez-Lira and Tang | Post-cutoff sample (Sept 2021 cutoff) | Retrospective scoring of headlines | Headlines | Gross positive, decays 6.54 to 1.22 |
| Choi et al. | Sample largely pre-dates Gemini-2.5-Pro; no post-cutoff test | Retrospective | Earnings-call transcripts | Gross positive, no cost, no turnover |
| Barton et al. | Live, Feb to Aug 2026 | Full telemetry, ClickHouse, per-turn joins | Agent tools, market context | No directional edge |
| Gencay | Point-in-time features; LLM proposer | Trial ledger of every evaluation | Price features only in headline runs | All discovered strategies fail |

- Sources for the table: [Chen and Pu](https://arxiv.org/abs/2601.11958); [DeepFund](https://arxiv.org/abs/2505.11065); [Lopez-Lira and Tang](https://arxiv.org/abs/2304.07619); [Choi et al.](https://arxiv.org/abs/2510.03195); [Barton et al.](https://arxiv.org/abs/2609.05663); [Gencay](https://arxiv.org/abs/2608.27734)
- Chen and Pu argue a later re-run cannot be reconstructed because later models embed post-period data and the retrieval environment is time-specific — [Chen and Pu](https://arxiv.org/abs/2601.11958)
- Deflation is not a substitute for leakage control: a planted oracle achieves DSR 1.00 — [Gencay](https://arxiv.org/abs/2608.27734)
- DeepFund frames its contribution as avoiding "time travel" by connecting to data published after each model's cutoff — [DeepFund](https://arxiv.org/abs/2505.11065)

### Inferences
- Choi's extraction task (reading a supplied transcript) is lower leakage risk than asking a model to forecast, since the model is not asked about outcomes. It is not leakage-free: Gemini-2.5-Pro post-dates nearly all of the 2010 to 2024 sample, and the thresholds were chosen with reference to predictive performance.
- Gencay's proposers (gpt-4.1, claude-sonnet-5) may know which strategy families performed over the evaluation window, which would bias results in favour of the LLM. The null result is therefore conservative on this dimension.
- For this project, any LLM-assisted backtest before about June 2026 is contaminated. Post-cutoff but already-observed periods (July to November 2026) are memory-free for the model, but the researcher has seen outcomes, so these are dry runs and not evidence.

### Gaps
- Candidate papers seen only in search results, not read and not counted: Lehner and Lopez-Lira, "ChatGPT as a Time Capsule" (arXiv 2604.21433, aggregator page only, which itself showed conflicting t-statistics); Look-Ahead-Bench (arXiv 2601.13770); DatedGPT (arXiv 2603.11838). Li et al. FINSABER is known only via citation in Gencay.
- I found no read paper reporting a live LLM signal on SEC filings specifically.

## Do these signals concentrate in small, illiquid names or persist in liquid stocks? How quickly do they decay?

### Takeaway
News-headline drift is strongest in small stocks and dies within about two days, and shows decaying Sharpe through 2024. The large-cap evidence is limited to Chen and Pu's short live sample and to gross S&P 100 and value-weighted filing results.

### Cited Findings
- Predictability is substantially stronger among smaller stocks: overnight drift regression has base coefficient 0.087 (t=4.09) and small-stock interaction 0.404 (t=4.75), with small defined as below the 20th NYSE size percentile — [Lopez-Lira and Tang](https://arxiv.org/abs/2304.07619)
- Following negative news, large-cap top-tercile stocks show no drift, whereas small caps continue to drift — [Lopez-Lira and Tang](https://arxiv.org/abs/2304.07619)
- After removing stocks at or below USD 5 and below the 20th NYSE size percentile, cumulative gross return is still over 300% (against about 700% for the full sample) — [Lopez-Lira and Tang](https://arxiv.org/abs/2304.07619)
- Speed: overnight strategy earns 34 bps on day 1 and 19 bps on day 2, then nothing significant; information is absorbed in about two days — [Lopez-Lira and Tang](https://arxiv.org/abs/2304.07619)
- Chen and Pu's alpha is concentrated at the top of the ranking, decaying to near zero by about ranks 80 to 120, and the bottom portfolios have no significant alpha. Signal horizons: monthly Sharpe 0.49 in text (0.52 in Table 5); quarterly alpha 0.12% in text (t=1.30, not significant) — [Chen and Pu](https://arxiv.org/abs/2601.11958)
- Lazy Prices returns accrue over months, to 18 months with no reversal, and show up in value-weighted returns — [Lazy Prices](https://www.nber.org/papers/w25084)
- Choi's result is on S&P 100 firms, so it is a large-cap result, but gross and equal-weight within that universe — [Choi et al.](https://arxiv.org/abs/2510.03195)
- McLean and Pontiff (2016), cited in Chen and Pu, is the standard reference for post-publication decay; I did not read it for this note — [Chen and Pu](https://arxiv.org/abs/2601.11958)

### Inferences
- Small-cap concentration conflicts with this account's cost structure: the size effect is where spreads are widest, and the drift is too short to amortize a USD 0.70 round trip.
- Headline signals (two-day half-life, about 190% daily turnover) are structurally unfit for a USD 2,460 account. Filing-diff signals (months of drift, quarterly formation) are the only type here whose horizon matches the commission constraint.
- The size-versus-small-cap question for filing signals is not settled by what I read: the value-weighted result is encouraging but dates from 1995 to 2014.

### Gaps
- No read paper gives a clean large-cap-only, post-cutoff, net-of-cost series.
- Post-2014 evidence for Lazy Prices was not read here.

## What can be done with SEC EDGAR filings alone, using filing timestamps, in a way that a model cannot have memorized?

### Takeaway
Use EDGAR's own acceptance timestamps, score only filings accepted after the registration date, and prefer a mechanical year-over-year filing diff (no model memory at all) as the primary arm, with an LLM-extractor port of moving targets as a secondary arm.

### Cited Findings
- EDGAR accepts filings 6:00 a.m. to 10:00 p.m. ET on business days; submissions begun after 5:30 p.m. ET (10:00 p.m. for Forms 3, 4, 5) are disseminated the next business day — [SEC, Accessing EDGAR Data](https://www.sec.gov/os/accessing-edgar-data)
- Fair-access limit is 10 requests per second, with a declared user-agent — [SEC, Accessing EDGAR Data](https://www.sec.gov/os/accessing-edgar-data)
- Daily and full indexes are available from 1994Q3; company submissions are served as JSON from data.sec.gov — [SEC, Accessing EDGAR Data](https://www.sec.gov/os/accessing-edgar-data)
- The submissions API is updated with a typical processing delay of under a second, longer at peak times, and requires no API key — [SEC, EDGAR APIs](https://www.sec.gov/search-filings/edgar-application-programming-interfaces)
- The submissions JSON `filings.recent` includes `accessionNumber`, `filingDate`, `reportDate`, `acceptanceDateTime` (for example "2026-11-06T21:30:47.000Z"), `form` and `items` — [data.sec.gov submissions, sample CIK](https://data.sec.gov/submissions/CIK0000320193.json)
- Year-over-year 10-K/10-Q comparisons need same-period documents, because prior-quarter comparisons are confounded by seasonality — [Lazy Prices](https://www.nber.org/papers/w25084)
- Choi's method compares each metric from the same fiscal quarter a year earlier and is stated as portable to annual reports and shareholder letters, but was only run on transcripts — [Choi et al.](https://arxiv.org/abs/2510.03195)

### Inferences
- Three separate leakage channels need separate controls. **Model memory:** only score filings accepted after the model's cutoff. **Selection memory:** this assistant proposed these rules with training data through about June 2026, so a historical pass on a rule designed now is not clean; only forward results after registration count. **Timing:** use acceptance and first-available trading session, not `filingDate` alone.
- A mechanical similarity score has no model-memory channel and can legitimately be backtested on filings before June 2026, but its hypothesis is published and known to the proposer, so a backtest is a replication and not a clean test. Treat it as a sanity check only.
- The `Z` suffix on `acceptanceDateTime` should not be trusted as UTC until verified: compare several values with the "Accepted" time on each filing's EDGAR index page before going live.
- Earnings-call transcripts are generally not on EDGAR, so a filings-only build should use 10-K and 10-Q MD&A text and 8-K exhibits. A transcript-based result does not automatically carry over to filings, so the port is a new hypothesis.

### Gaps
- I did not verify the time zone of `acceptanceDateTime`.
- Availability of 8-K press-release exhibits and their usability as earnings-call substitutes was not tested.
- I did not verify IBKR fractional-share eligibility for the universe.

## Exact rule definitions for a forward, pre-registered test, including how the score is logged before outcomes exist

### Takeaway
Register at most two arms, freeze every parameter and model ID, hash-chain each score to a public anchor before formation, trade quarterly in five names, and pre-commit that a realistic forward window is too short to confirm a moderate edge.

### Cited Findings
- A pre-registered single hypothesis faces almost no deflation (at N=1, DSR equals PSR); the bar rises with each recorded variant — [Gencay](https://arxiv.org/abs/2608.27734)
- A t-statistic on a Sharpe ratio grows as Sharpe times the square root of years; a true Sharpe of 0.6 needs roughly eleven years to reach t of about 2 — [Gencay](https://arxiv.org/abs/2608.27734)
- The market day should be the inferential unit, since position-level intervals ran about 2.5 times too narrow; a repeatedly read rolling p-value is not a p-value; agent-reported P&L is non-canonical and reconstructed fills should be used; a null arm should be calibrated first — [Barton et al.](https://arxiv.org/abs/2609.05663)
- Single-query LLM scores correlated only 0.629 with the mean of repeated queries, so a single draw is noisy — [Chen and Pu](https://arxiv.org/abs/2601.11958)
- Choi's prompt, extraction schema and thresholds (alpha 0.4, beta 0.6) are published in the paper — [Choi et al.](https://arxiv.org/abs/2510.03195)
- Lazy Prices forms portfolios the month after release, using year-ago comparisons — [Lazy Prices](https://www.nber.org/papers/w25084)

### Inferences (proposed rules; these are my design, untested)

**Common to both arms**
- Universe, frozen at each formation date: US-listed common stock; top 1,000 by market cap from a point-in-time source; price at or above USD 10; 63-day median dollar volume at or above USD 20m; not in a merger or tender process per that date's 8-K check.
- Formation dates: last trading day of March, June, September, December, which line up with the main filing seasons in Lazy Prices.
- Eligible filing: original 10-K or 10-Q (no amendments, no NT forms), accepted during that calendar quarter and no later than 16:00 ET on the session before formation, with a same-form same-fiscal-period filing from the same CIK accepted 330 to 400 days earlier.
- Portfolio: long only, top 5 names, equal weight, no leverage, cash for unfilled slots. Tie-break by larger market cap (following Chen and Pu's tie-break convention).
- Execution: market-on-open at the next session after formation; hold until the next formation's open; replace only names leaving the top 5, so unchanged names trade nothing.
- Cost model per fill: maximum of USD 0.35 and actual commission, plus half the quoted spread at the prior close, plus 5 bps slippage. Doubled-cost test: multiply all three by 2.
- Benchmark: equal-weight portfolio of all eligible universe names at each formation, same dates, same cost model.
- Opposite-side control: long-only paper portfolio of the bottom 5 names. It must fail the bar.
- Random control: 5 names chosen by a seeded random draw from the eligible set, logged at the same time, as Barton's null-arm rule requires.

**Arm A, primary: mechanical filing similarity (no LLM)**
- Text: download the primary document, strip HTML, tables and exhibits; lowercase; keep alphabetic tokens.
- Score: cosine similarity of term-frequency vectors between the current filing and the year-ago filing. Higher means fewer changes.
- Selection: top 5 by score. Record N=4 in the ledger, since cosine was chosen from Lazy Prices' four measures after seeing their long-leg t-statistics.

**Arm B, secondary: LLM-extractor moving targets on filings (forward-only)**
- Text: MD&A section only (10-K Item 7; 10-Q Part I Item 2). Year-ago MD&A from the same comparison filing.
- Extractor: Choi's Table 5 prompt, adapted only to replace the transcript with MD&A and drop the presentation and Q&A split; temperature 0; three repeated extractions, retaining metrics appearing in at least two.
- Ruler: pinned embedding model; thresholds alpha 0.4, beta 0.6 as published.
- Score: MT = 1 - mean(S). Selection: lowest 5 MT.
- Pin the exact model identifier. A retired or changed model is a new arm and restarts the clock; this avoids Chen and Pu's "always the latest version" confound.
- Only filings accepted after the registration date count.

**Logging before outcomes exist**
- Append-only JSONL, one record per filing scored: `run_id`, `accession_number`, `cik`, `form`, `acceptance_datetime_raw`, `fetched_at_utc`, `source_url`, `doc_sha256`, `prior_accession`, `prior_doc_sha256`, `method_git_commit`, `model_id`, `embedding_model_id`, `prompt_sha256`, `temperature`, `n_repeats`, `raw_response_sha256`, `score`, `rank`, `formation_date`, `logged_at_utc`, `prev_line_sha256`.
- Before each formation date: compute the digest of the full file, create a signed git tag, push it to a remote, and record the digest and its timestamp externally. A score counts only if `logged_at_utc` is later than the acceptance time and earlier than the formation cut-off, and the anchor predates the first trade.
- Separately log target positions with the same digest method before market open, then log reconstructed fills from broker statements (never model-reported P&L).
- Keep a trial ledger of every variant ever evaluated, including discarded ones. Any parameter change creates a new arm with N incremented and a new clock.

**Decision rule, fixed in advance**
- Pass requires all of: net of costs positive; positive at doubled costs; weekly-cluster bootstrap lower bound above zero (10,000 draws, weekly blocks, one-sided 2.5% on weekly returns net of benchmark); no negative calendar year; beats the equal-weight benchmark; bottom-5 arm fails.
- Evaluation dates fixed at each December 31 and no interim reads. Pre-commit a minimum of 104 weeks before any judgement.
- Power, my arithmetic: at active Sharpe 0.5, reaching t of about 2 needs about 16 years ((2/0.5) squared), so a two-year forward window will probably be inconclusive even if the edge is real. Inconclusive means no capital increase, and it should not be read as confirmation.

**Rejected for this account:** daily headline signals (two-day decay, about 190% daily turnover), daily Top-20 agentic nowcast (commission alone exceeds alpha), and anything requiring short legs, where most of the Lopez-Lira and Tang profit sits.

### Gaps
- Rule parameters (USD 10 price floor, USD 20m liquidity floor, 330 to 400 day window, 5 names, 5 bps slippage) are design choices of mine, not taken from any paper.
- Selecting 5 names from a top-quintile signal is untested; Lazy Prices documents quintile portfolios, not five-stock portfolios, so idiosyncratic risk will dominate realized results.
- The merger-screen implementation was not specified further or tested.
- Arm B's prompt port to MD&A has no published validation.
