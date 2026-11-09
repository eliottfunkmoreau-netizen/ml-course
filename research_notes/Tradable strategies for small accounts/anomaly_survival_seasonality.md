# Equity anomaly survival, net-of-cost results, and calendar or structural effects for a USD 2,500 long-only account

Reading status (seven papers, the maximum). All seven were read from a PDF. Text-layer PDFs were extracted with pypdf, and the McConnell and Xu scan was read as page images.

| Paper | Version read | How much was read |
|---|---|---|
| McLean and Pontiff | "Journal of Finance, Forthcoming" manuscript | Text extracted; abstract, intro, sample and cost sections read, rest keyword-searched |
| Hou, Xue, Zhang | NBER WP 23394 (May 2017, 447 anomalies). The published RFS version (452 anomalies) was NOT opened | Text extracted; intro, profitability passages, appendix definitions read, rest keyword-searched |
| Chen and Zimmermann | FEDS 2021-037 (March 2021) | Text extracted; abstract, intro, MP replication, rebalancing passages read |
| Jensen, Kelly, Pedersen | NBER WP 28432 (Feb 2021). Published JF version NOT opened | Text extracted; intro, construction, size and cluster, out-of-sample passages read |
| Novy-Marx and Velikov | NBER WP 20721 (Dec 2014). Published RFS version NOT opened | Text extracted; intro, Tables 2 and 3, buy/hold section, appendix definitions read |
| McConnell and Xu | Published FAJ 2008 scan | Pages 49-53 and 63 of the article read as images. Pages 54-62 (Tables 2 onward, country and volume tests) NOT viewed |
| Lou, Polk, Skouras | FMG Discussion Paper 744 (March 2015 draft). Published JFE version timed out and was NOT read | Text extracted. The extraction renders "%" as "3" (for example "0.953" is 0.95%); figures below are corrected for that |

Anything else below is labeled as an abstract, a search snippet, or a secondary source, and does not count as a paper read. Novy-Marx (2013), Asness-Frazzini-Pedersen, Fama-French (2015), Lakonishok-Smidt were not read directly because of the seven-paper cap; they appear only as described by the papers above.

## For each paper: citation, test, sample, net-of-cost result, turnover, out-of-sample survival, and USD 2,500 capacity or cost issues

### Takeaway
None of the four replication papers reports net-of-cost returns; only Novy-Marx and Velikov does, and it shows that annually rebalanced profitability and value-profitability sorts survive costs while strategies turning over more than 50% a month almost never do. For a USD 2,500 account the binding constraint is the fixed USD 0.35 per order, not market impact.

### Cited Findings
**1. McLean and Pontiff, "Does Academic Research Destroy Stock Return Predictability?", Journal of Finance 2016, DOI 10.1111/jofi.12365.**
- Tested 97 published cross-sectional predictors over three windows: original sample, post-sample pre-publication, post-publication. Sample ends 2013; the time variable starts January 1926; the average original sample is 323 months and the average post-publication span 156 months — [McLean and Pontiff](https://counterpointfunds.com/wp-content/uploads/2017/07/PredictabilityMcleanPontiff.pdf)
- Average in-sample long-short return 58.2 bps per month; returns are 26% lower post-sample and 58% lower post-publication. The 26% is an upper bound on data mining and 32% (58 minus 26) is the estimated publication-informed trading effect — [McLean and Pontiff](https://counterpointfunds.com/wp-content/uploads/2017/07/PredictabilityMcleanPontiff.pdf)
- Net of cost: not reported. The returns are gross long-short, and arbitrage costs enter only as cross-sectional explanatory variables — [McLean and Pontiff](https://counterpointfunds.com/wp-content/uploads/2017/07/PredictabilityMcleanPontiff.pdf)
- Decay is larger for predictors with higher in-sample returns and t-statistics, and for predictors built only from price and trading data. Post-publication returns are lower for portfolios in liquid, low-idiosyncratic-risk stocks — [McLean and Pontiff](https://counterpointfunds.com/wp-content/uploads/2017/07/PredictabilityMcleanPontiff.pdf)

**2. Hou, Xue, Zhang, "Replicating Anomalies", Review of Financial Studies 33(5), 2020, 2019-2133 ([journal listing](https://econpapers.repec.org/RePEc:oup:rfinst:v:33:y:2020:i:5:p:2019-2133.)).**
- WP version: 447 anomalies, deciles on NYSE breakpoints with value-weighted returns to limit microcaps. 286 (64%) are insignificant at 5%; 380 (85%) fail a t of 3. 95 of 102 liquidity variables (93%) fail — [HXZ NBER WP](https://www.nber.org/system/files/working_papers/w23394/w23394.pdf)
- The published version's headline, as quoted by Chen and Zimmermann, is that 65.4% fail |t| >= 1.96 on the original shorter samples, and that the failure rate drops to 43.1% with NYSE-Amex-NASDAQ breakpoints and equal weights — [Chen and Zimmermann](https://www.federalreserve.gov/econres/feds/files/2021-037pap.pdf)
- Results are gross; no net-of-cost or turnover results were found in the sections read. Microcaps are 3% of market cap but 60% of the number of stocks — [HXZ NBER WP](https://www.nber.org/system/files/working_papers/w23394/w23394.pdf)

**3. Chen and Zimmermann, "Open Source Cross-Sectional Asset Pricing", Critical Finance Review 11(2), 2022, 207-264 ([listing](https://econpapers.repec.org/RePEc:now:jnlcfr:104.00000112)).**
- 319 characteristics. For the 161 clearly significant originally, 98% of long-short reproductions have t above 1.96. Regressing reproduced on original t-statistics gives slope 0.90, R-squared 83% — [Chen and Zimmermann](https://www.federalreserve.gov/econres/feds/files/2021-037pap.pdf)
- Value-weighting or dropping stocks below the NYSE 20th percentile lowers in-sample mean returns by about 30% (20 bps per month) — [Chen and Zimmermann](https://www.federalreserve.gov/econres/feds/files/2021-037pap.pdf)
- Returns decay post-publication but stay positive, quantitatively like McLean and Pontiff, including for predictors McLean and Pontiff did not study — [Chen and Zimmermann](https://www.federalreserve.gov/econres/feds/files/2021-037pap.pdf)
- Net of cost: explicitly not addressed. The authors warn they do not test implementable profits and cite Chen and Velikov (2019), who find effective spreads wipe out most post-publication returns across a large anomaly set (cited via Chen and Zimmermann; Chen and Velikov not read) — [Chen and Zimmermann](https://www.federalreserve.gov/econres/feds/files/2021-037pap.pdf)

**4. Jensen, Kelly, Pedersen, "Is There a Replication Crisis in Finance?", NBER WP 28432, 2021 (later Journal of Finance; volume and pages not verified this session).**
- 153 factors, 93 countries, 13 theme clusters. Tercile long-short with breakpoints from stocks above the NYSE 20th percentile, capped value weights (market equity winsorized at NYSE 80th percentile), accounting data always lagged four months, one-month holding — [JKP](https://www.nber.org/system/files/working_papers/w28432/w28432.pdf)
- US data from CRSP starting in 1926 — [JKP](https://www.nber.org/system/files/working_papers/w28432/w28432.pdf)
- Overall US empirical-Bayes replication rate 84.0%. By size: mega 77.3%, large 81.5%, micro 81.5%, nano 71.4%. HXZ's 35% is the starting point in their comparison — [JKP](https://www.nber.org/system/files/working_papers/w28432/w28432.pdf)
- Average alpha 0.45% per month in-sample and 0.31% post-original-sample, a decline of about one third. 82.6% of factors are positive post-original-sample. Post-sample alpha regressed on in-sample alpha has slope 0.25 (t range across panels 2.9 to 4.6) — [JKP](https://www.nber.org/system/files/working_papers/w28432/w28432.pdf)
- Net of cost: not reported in the sections read (gross factor alphas) — [JKP](https://www.nber.org/system/files/working_papers/w28432/w28432.pdf)

**5. Novy-Marx and Velikov, "A Taxonomy of Anomalies and Their Trading Costs", NBER WP 20721, 2014 (later Review of Financial Studies; volume and pages not verified this session).**
- 23 anomalies, value-weighted decile long-short with NYSE breakpoints, 07/1963 (or 07/1973) to 12/2012, costs from Hasbrouck effective spreads (no price impact, so a lower bound) — [NMV](https://www.nber.org/system/files/working_papers/w20721/w20721.pdf)
- Round-trip cost for typical value-weighted strategies averages over 50 bps; equal-weighted strategies cost two to three times as much. Only two strategies with more than 50% one-sided monthly turnover have significant net spreads — [NMV](https://www.nber.org/system/files/working_papers/w20721/w20721.pdf)
- Table 3 (monthly %, one-sided turnover %): Gross Profitability gross 0.40 [2.94], turnover 1.96, cost 0.03, net 0.37 [2.74], FF4 net alpha 0.51 [3.77] — [NMV](https://www.nber.org/system/files/working_papers/w20721/w20721.pdf)
- ValProf (rank sum of book-to-market and gross profitability): gross 0.82 [5.18], turnover 2.94, cost 0.06, net 0.77 [4.82], FF4 net alpha 0.49 [3.93] — [NMV](https://www.nber.org/system/files/working_papers/w20721/w20721.pdf)
- Value net 0.42 [2.39]; Investment net 0.46 [3.60]; Accruals net 0.18 [1.43]; Asset Growth net 0.26 [1.75]; Piotroski F-score gross 0.20 [1.04], net 0.09 [0.45] — [NMV](https://www.nber.org/system/files/working_papers/w20721/w20721.pdf)
- Higher-turnover: Return-on-book equity gross 0.71, turnover 22.27, cost 0.38, net 0.33 [1.38]; Momentum net 0.68 [2.45] at 34.52 turnover; Idiosyncratic Volatility net 0.11 [0.37]; Short-run Reversals gross 0.37, cost 1.65, net -1.28 [-6.02]; Seasonality (cross-sectional, not calendar) gross 0.84, net -0.62 [-3.88] — [NMV](https://www.nber.org/system/files/working_papers/w20721/w20721.pdf)
- A 10%/20% buy/hold band is the most effective simple mitigation: Momentum turnover falls to 18.82 and net rises to 0.85 [3.35]; ValMomProf net 1.02 [6.19] at 11.57 turnover — [NMV](https://www.nber.org/system/files/working_papers/w20721/w20721.pdf)
- Disclosure: Novy-Marx consults for Dimensional Fund Advisors, and profitability is his own published signal — [NMV](https://www.nber.org/system/files/working_papers/w20721/w20721.pdf)

**6. McConnell and Xu, "Equity Returns at the Turn of the Month", Financial Analysts Journal 64(2), 2008, 49-64.**
- CRSP VW and EW indices, 1926-2005, and 35 countries. Day -1 is the last trading day of the prior month; the window is Day -1 to Day +3 — [McConnell and Xu](https://apps.it.purdue.edu/sites/Home/DirectoryApi/Files/fed4cdba-2490-4cc5-893d-9f0d5efb626f/Download)
- Mean daily returns, 1987-2005 (post-discovery): VW 0.15% (t 4.35) in the window versus -0.00% on other days (t -0.07); difference t 3.78, positive in 61% of months. EW 0.25% (t 7.58) versus 0.05%. Subperiods: 0.17 (t 3.63) for 1987 to mid-1996 and 0.14 (t 2.00) from mid-1996 to 2005 — [McConnell and Xu](https://apps.it.purdue.edu/sites/Home/DirectoryApi/Files/fed4cdba-2490-4cc5-893d-9f0d5efb626f/Download)
- 1926-1986 VW 0.16% versus 0.01%; the largest size decile 0.15% versus 0.01% (difference t 7.81); smallest decile 0.25% versus 0.03% (t 8.54). Found in 30 of 34 non-US countries — [McConnell and Xu](https://apps.it.purdue.edu/sites/Home/DirectoryApi/Files/fed4cdba-2490-4cc5-893d-9f0d5efb626f/Download)
- Net of cost: no cost analysis in the abstract, conclusion, or pages read; returns are index returns. Turnover is one round trip per month — [McConnell and Xu](https://apps.it.purdue.edu/sites/Home/DirectoryApi/Files/fed4cdba-2490-4cc5-893d-9f0d5efb626f/Download)

**7. Lou, Polk, Skouras, "A Tug of War: Overnight Versus Intraday Expected Returns", Journal of Financial Economics 134 (2019) 192-213; read as FMG DP744, 2015.**
- Cross-sectional value-weighted deciles with NYSE breakpoints, 1993-2013, excluding stocks under USD 5 and bottom NYSE size quintile; open prices checked with first-half-hour VWAP and quote midpoints — [Lou, Polk, Skouras](https://www.fmg.ac.uk/sites/default/files/publications/DP744.pdf)
- Momentum three-factor alpha 1.05% per month (t 2.22) of which 0.95% (t 3.65) is overnight and 0.11% (t 0.27) intraday — [Lou, Polk, Skouras](https://www.fmg.ac.uk/sites/default/files/publications/DP744.pdf)
- Profitability is the reverse: high-minus-low intraday three-factor alpha 1.43% (t 6.44), overnight -0.95% (t -6.22) — [Lou, Polk, Skouras](https://www.fmg.ac.uk/sites/default/files/publications/DP744.pdf)
- Net of cost: not reported in this draft's sections read. These are long-short decile decompositions, not a tradable overnight account strategy — [Lou, Polk, Skouras](https://www.fmg.ac.uk/sites/default/files/publications/DP744.pdf)

### Inferences
- Market impact is irrelevant at USD 2,460. Fixed commission is what matters: USD 0.35 on a USD 246 position (10 equal names) is 14.2 bps per order, or 28.5 bps per full round trip, while the same order on the whole account is 1.4 bps. Own arithmetic from the stated USD 0.35 minimum.
- The published institutional cost figures (3 to 6 bps per month for low-turnover sorts) probably understate this account's cost on small positions and overstate its spread cost in liquid names. Costs must be modeled from the account's own commission schedule, not copied from NMV.
- The prior rejections on your data are consistent with this literature. Reversal, idiosyncratic volatility and high-turnover signals are the ones NMV shows failing net of cost, and HXZ lists short-term reversal, volatility and turnover-style liquidity variables among the failures.

### Gaps
- Published-version numbers for HXZ, JKP, NMV and LPS may differ from the working papers read. HXZ in particular moved from 447 to 452 anomalies.
- No paper here measures net results for a long-only, 10-stock, USD 2,500 implementation.

## What McLean and Pontiff, Hou Xue Zhang, Chen and Zimmermann, and Jensen Kelly Pedersen conclude, and how much haircut to apply

### Takeaway
The four papers disagree mainly on the replication hurdle, not on the direction of decay: returns shrink roughly one third (JKP) to about 58% (McLean and Pontiff) after the original sample, and value-weighting removes a further 30%. Use a 50 to 60% haircut on any published gross long-short spread before costs, and more for signals that are price-only or concentrated in illiquid names.

### Cited Findings
- McLean and Pontiff: predictability declines but does not disappear; they reject both zero post-publication predictability and no change — [McLean and Pontiff](https://counterpointfunds.com/wp-content/uploads/2017/07/PredictabilityMcleanPontiff.pdf)
- HXZ: "widespread p-hacking"; the 65% failure is driven by microcaps being limited; even significant anomalies are often much smaller than originally reported — [HXZ NBER WP](https://www.nber.org/system/files/working_papers/w23394/w23394.pdf)
- Chen and Zimmermann: the literature is reproducible, and the HXZ failure rate partly reflects including characteristics that were never clearly significant originally — [Chen and Zimmermann](https://www.federalreserve.gov/econres/feds/files/2021-037pap.pdf)
- JKP: most factors replicate and post-sample decay is what Bayesian updating predicts. HXZ's microcap argument is rejected by similar replication in mega and large stocks — [JKP](https://www.nber.org/system/files/working_papers/w28432/w28432.pdf)
- 10 of 13 JKP US themes replicate at 75% or better. The exceptions are seasonality, leverage and size, so profitability and quality are among the themes above 75% — [JKP](https://www.nber.org/system/files/working_papers/w28432/w28432.pdf)
- Net of costs is the main unresolved point: Chen and Velikov (via Chen and Zimmermann) find effective spreads wipe out most post-publication returns — [Chen and Zimmermann](https://www.federalreserve.gov/econres/feds/files/2021-037pap.pdf)

### Inferences
- Recommended haircut schedule for planning:
  - Well-replicated, fundamental, low-turnover, large-cap factor (GP/A, value-profitability): 50 to 60% off published gross L/S.
  - Price-only signals: 70% or more, since McLean and Pontiff find decay is larger for them.
  - Equal-weighted or small-cap-driven published results: first remove about 30% (Chen and Zimmermann), then apply the above.
- Worked example, GP/A: 0.40% per month gross L/S times 0.42 (a 58% haircut) is about 0.17% per month, or 2.0% per year L/S. A long-only leg captures only part of that. Assuming half (assumption, not sourced), the expected active return is about 1% per year against costs near 0.1 to 0.3% per year. Own arithmetic.
- At a 1% expected active return and a 10-stock portfolio, a backtest on roughly 14 annual formations will have low statistical power. A positive bootstrap lower bound would be a surprise, and a failure would not disprove the effect.

### Gaps
- No read paper splits post-publication decay for profitability or quality specifically.
- JKP's exact cluster replication percentages appear only in a figure and could not be read from the extracted text, beyond the 75% statement.

## Do quality or profitability tilts work net of cost in large and mid caps, and can they be built from free SEC EDGAR XBRL fundamentals point-in-time?

### Takeaway
Gross profitability and value-profitability are the best-supported net-of-cost effects here, on value-weighted NYSE-breakpoint sorts, though those are long-short decile results that do not show the long-only leg. EDGAR XBRL can support them from about FY2011 onward, but the frames API is not point-in-time and must be avoided.

### Cited Findings
- Gross profitability net 0.37% per month [2.74], annual rebalance, 1.96% turnover, excluding SIC 6000-6999 — [NMV](https://www.nber.org/system/files/working_papers/w20721/w20721.pdf)
- HXZ replicates Gpa at 0.38% per month (t 2.62) — [HXZ NBER WP](https://www.nber.org/system/files/working_papers/w23394/w23394.pdf)
- Skeptical: gross profits to lagged assets (Gla) earns only 0.16% (t 1.04). HXZ argues Gpa is confounded with investment because its denominator is current assets — [HXZ NBER WP](https://www.nber.org/system/files/working_papers/w23394/w23394.pdf)
- Skeptical: HXZ lists the Fama-French (2015) operating profits-to-book equity among prominent insignificant variables — [HXZ NBER WP](https://www.nber.org/system/files/working_papers/w23394/w23394.pdf)
- HXZ lists cash-based operating profits-to-assets among anomalies whose q-factor alpha remains significant — [HXZ NBER WP](https://www.nber.org/system/files/working_papers/w23394/w23394.pdf)
- Piotroski F-score fails net of cost in NMV: net 0.09 [0.45] — [NMV](https://www.nber.org/system/files/working_papers/w20721/w20721.pdf)
- Replication among large stocks is close to the full sample: mega 77.3%, large 81.5% — [JKP](https://www.nber.org/system/files/working_papers/w28432/w28432.pdf)
- Profitability is earned intraday and is negative overnight in 1993-2013 — [Lou, Polk, Skouras](https://www.fmg.ac.uk/sites/default/files/publications/DP744.pdf)
- Definitions: Gpa is REVT minus COGS over current AT, sorted each June on fiscal year ending in calendar t-1, held July to June — [HXZ NBER WP](https://www.nber.org/system/files/working_papers/w23394/w23394.pdf)
- SEC APIs: companyconcept and companyfacts return a company's XBRL facts without an API key, and companyfacts.zip is rebuilt nightly. Frames returns one fact per entity, the "last filed" one fitting the period, so it embeds later revisions — [SEC API documentation](https://www.sec.gov/search-filings/edgar-application-programming-interfaces)
- XBRL phase-in: filers with public float above USD 5 billion for periods ending on or after June 15, 2009; other large accelerated filers from June 15, 2010; all others from June 15, 2011 (search-result summary of the SEC release, page not opened) — [Cooley](https://www.cooley.com/news/insight/2009/sec-posts-mandatory-xbrl-release)

### Inferences
- Yes, with limits. A June 2012 formation is the first with full coverage of FY2011 across filers, giving about 14 annual formations through 2026. Own inference from the phase-in dates.
- Point-in-time rule: use companyfacts only, keep the earliest-filed value for each CIK and period end, and require filed date before the formation date. Do not use frames for backtests.
- Field names from prior knowledge, verify against live JSON because the SEC page fetched did not list them: `end`, `val`, `accn`, `fy`, `fp`, `form`, `filed`, `frame`.
- Tags from prior knowledge, verify coverage: `GrossProfit`; revenue via `Revenues`, `RevenueFromContractWithCustomerExcludingAssessedTax`, `SalesRevenueNet`; cost via `CostOfRevenue`, `CostOfGoodsAndServicesSold`, `CostOfGoodsSold`; `Assets`; `StockholdersEquity`.
- EDGAR filings remain available for delisted filers, but free price sources often lose delisted tickers. Survivorship bias is likelier to come from the price feed than from fundamentals.

### Gaps
- No read source tests a long-only top-decile profitability portfolio net of costs.
- No read source tests profitability built from XBRL rather than Compustat, or quantifies missing cost-of-revenue tags. Firms with no cost-of-revenue tag will be dropped, possibly non-randomly.
- Asness-Frazzini-Pedersen quality-minus-junk and Novy-Marx (2013) were not read because of the seven-paper cap.

## Do turn-of-month or overnight effects survive after costs when traded with a few ETFs or stocks at USD 0.35 minimum commission, including skeptical evidence?

### Takeaway
Turn-of-month has a strong historical gross record and costs about 0.34% per year on one ETF, so commission does not kill it, but the evidence read stops at 2005 and it may fail the equal-weight benchmark bar because it sits in cash about 80% of days. Daily overnight holding costs about 7.2% per year in commissions and is effectively ruled out at this account size.

### Cited Findings
- Turn-of-month 1987-2005 VW 0.15% per day over four days versus about zero on other days — [McConnell and Xu](https://apps.it.purdue.edu/sites/Home/DirectoryApi/Files/fed4cdba-2490-4cc5-893d-9f0d5efb626f/Download)
- The effect persists in the largest size decile, which supports trading it with a large-cap ETF — [McConnell and Xu](https://apps.it.purdue.edu/sites/Home/DirectoryApi/Files/fed4cdba-2490-4cc5-893d-9f0d5efb626f/Download)
- Skeptical: Maberly and Waggoner (2000) found S&P 500 futures turn-of-month disappeared after 1990; McConnell and Xu explicitly disagree using CRSP cash indices through 2005 — [McConnell and Xu](https://apps.it.purdue.edu/sites/Home/DirectoryApi/Files/fed4cdba-2490-4cc5-893d-9f0d5efb626f/Download); [Atlanta Fed listing](https://www.atlantafed.org/research/publications/wp/2000/11.aspx)
- Favorable, abstract only: Zwergel (2010) reports an out-of-sample turn-of-month strategy profitable in the futures studied including costs and slippage. PDF not read — [Applied Financial Economics listing](https://ideas.repec.org/a/taf/apfiec/v20y2010i11p911-922.html)
- Secondary and paywalled: a SPY strategy long close-to-open only during the turn of the month earned "0.0% net CAGR" with -39% max drawdown at 0.06% friction per round trip, Feb 1993 to Feb 2022 (eight round trips per month). Original not verified — [CXO Advisory](https://www.cxoadvisory.com/guides/intraday-overnight/)
- Secondary and paywalled excerpts: the NightShares overnight ETFs NSPY and NIWM trailed SPY and IWM from June 28, 2022 to July 31, 2023; the overnight-minus-intraday gap in index ETFs weakened over the prior 5 to 10 years (1995 to May 2022, gross) — [CXO Advisory](https://www.cxoadvisory.com/guides/intraday-overnight/)
- Search snippets only: an Alpha Architect test of SPY close-to-open (1993-2020) reports low statistical reliability after costs, but its page returned HTTP 403 and could not be read — [Alpha Architect](https://alphaarchitect.com/trading-costs-wipe-out-the-overnight-return-anomaly/)
- Lou, Polk, Skouras is a cross-sectional long-short decomposition that needs shorting and daily rebalancing, and it gives no ETF-level net result — [Lou, Polk, Skouras](https://www.fmg.ac.uk/sites/default/files/publications/DP744.pdf)

### Inferences
- Commission arithmetic on USD 2,460 (own calculation): one ETF round trip is USD 0.70, or 2.85 bps.
  - Turn-of-month, 12 round trips per year: USD 8.40, 0.34% per year; doubled, 0.68%.
  - McConnell-Xu gross is about 0.60% per month (0.15% times four days), or about 7.2% per year simple. Commission is about 5% of that gross figure.
  - Daily overnight, 252 round trips: USD 176.40, 7.17% per year; doubled, 14.3%. No backtest is likely to clear positive-at-doubled-costs unless overnight gross exceeds about 14% per year.
- Turn-of-month and the evaluation bar conflict in one place: holding about 4 of 21 days means it can only beat a buy-and-hold equal-weight benchmark in samples where other days earn about zero. Post-2005 other-day returns were not verified here, and in a strong bull market the strategy may lose to the benchmark while still earning a real per-day premium. Pre-register whether "beats benchmark" means total return or return per day invested.
- Cost-free use of the effect: if a stock tilt is rebalanced anyway, schedule trades so that new longs are in place by the Day -2 close. Not tested by any paper read.

### Gaps
- No read paper tests turn-of-month after 2005 on SPY or RSP net of costs. This must come from your own backtest.
- ETF spreads and MOC/MOO auction slippage were not sourced; the spread assumption below is not from a read paper.
- I found no primary paper with a verified ETF-level overnight net-of-cost result. Primary overnight-drift literature such as Cooper-Cliff-Gulen and Boyarchenko et al was not read.

## Exact rule definitions for a pre-registered backtest

### Takeaway
Register three hypotheses, in this order of priority: ValProf long-only, GP/A long-only, and SPY turn-of-month. Treat daily overnight as a kill test only, and correct for three primary tests.

### Cited Findings
- Signals and rebalance timing follow NMV and HXZ: GP/AT excluding SIC 6000-6999, annual, end of June, fiscal year ending in calendar t-1 — [NMV](https://www.nber.org/system/files/working_papers/w20721/w20721.pdf); [HXZ NBER WP](https://www.nber.org/system/files/working_papers/w23394/w23394.pdf)
- ValProf is the sum of a firm's ranks on book-to-market and gross profitability — [NMV](https://www.nber.org/system/files/working_papers/w20721/w20721.pdf)
- The hysteresis band follows the NMV sS rule: buy on entering the top 10%, hold until leaving the top 20% — [NMV](https://www.nber.org/system/files/working_papers/w20721/w20721.pdf)
- Non-micro universe per JKP (above NYSE 20th percentile) — [JKP](https://www.nber.org/system/files/working_papers/w28432/w28432.pdf)
- Turn-of-month window: Day -1 (last trading day of month) through Day +3, close-to-close — [McConnell and Xu](https://apps.it.purdue.edu/sites/Home/DirectoryApi/Files/fed4cdba-2490-4cc5-893d-9f0d5efb626f/Download)

### Inferences
The parameters below are proposed specifications. Those not attributed above are my choices and should be fixed before seeing data.

**Shared universe (H1 and H2).** US-listed common stocks on NYSE, NASDAQ or NYSE American; exclude SIC 6000-6999, ETFs, ADRs and any security without complete data. Market cap in the top 1,000 at formation (a proxy for non-micro, since NYSE breakpoints need a full NYSE list). Price at least USD 5. 60-day median dollar volume at least USD 20 million.

**H1 ValProf long-only (primary).**
- Formation date F: last NYSE trading day of June, each year from 2012.
- Data rule: first-reported 10-K values (earliest `filed` per CIK and period end), `filed` strictly before F, fiscal year ending in calendar year t-1.
- GP = `GrossProfit` if present, else revenue minus cost of revenue. AT = `Assets`. GPA = GP / AT.
- BM = `StockholdersEquity` at fiscal year end divided by market cap at the last December close (use the filing's shares and split-adjusted price). Exclude BE at or below 0.
- Score = rank(BM) + rank(GPA), both ascending within universe.
- Portfolio: equal weight 10 names; start with the top 10 by score. Thereafter, each F, sell only holdings outside the top 20% of score, and fill empty slots with the highest-score names in the top 10% that are not held.
- Execution: signal at F close, orders at the next session's open. Fractional shares allowed.

**H2 GP/A long-only.** Same as H1 with Score = rank(GPA). Disclosed robustness only, not a pass criterion: GP over lagged assets, because HXZ reports that variant is insignificant.

**H3 Turn-of-month.**
- Instrument: SPY. Secondary reported, not a pass criterion: RSP, because McConnell and Xu find a larger EW effect.
- Using the NYSE calendar, Day -1 is the month's last trading day and Day +3 is the next month's third trading day.
- Buy MOC at the close of the trading day before Day -1 (Day -2). Sell MOC at the close of Day +3. This captures the four close-to-close returns of Days -1, +1, +2, +3.
- Flat otherwise. Assume 0% on cash in the base case.
- Sample: full SPY history with a separate 2006-to-date report, since that is outside McConnell and Xu.
- Opposite side: long from Day +3 close to the next Day -2 close. It must not pass the bar.

**Kill test: daily overnight.** Buy SPY MOC every session, sell MOO the next session. Cost 2 orders per night. Reported only to document the failure unless gross exceeds about 14% per year.

**Cost model (all).** Commission USD 0.35 per order, on the USD 2,460 account, compounding. Add a half-spread assumption of 5 bps per side for H1 and H2 and 1 bp per side for SPY (my assumption, unsourced). Doubled-cost test doubles both.

**Evaluation (as you specified, with precise definitions to fix).** Net of costs and at doubled costs positive; weekly-cluster bootstrap 95% lower bound of mean net weekly return above zero; no negative year; beats equal-weight benchmark; opposite side fails. Opposite side for H1 and H2 is the 10 lowest-score names in the same universe. Decide before running whether "no negative year" applies to absolute returns or to active returns versus equal weight: a fully invested long-only stock portfolio is likely to fail the absolute version in a year like 2022 regardless of signal quality. Apply a Bonferroni threshold across the three primary hypotheses.

**Expectation.** H1 and H2 have the strongest evidence of survival net of costs but low power on about 14 annual formations. H3 has the strongest gross calendar record but depends on post-2005 behavior not verified here. Neither is assured to pass the full bar.

### Gaps
- Cash yield, dividend handling on stocks held and the exact equal-weight benchmark universe are left to implementation and should be fixed before running.
- The half-spread values and USD 20 million volume filter are unsourced design choices.
- IBKR commission per-share tiers above the stated minimum were not checked; only the USD 0.35 minimum you gave is used.
