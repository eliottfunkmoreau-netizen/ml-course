# Event-driven US equity strategies for a USD 2,500 retail account

Read status, stated plainly. Eight papers were read as extracted PDF text, by abstract, introduction and targeted passages rather than every table: Martineau; Heitz et al.; Frazzini and Lamont; Cohen, Malloy and Pomorski; Jeng, Metrick and Zeckhauser; Greenwood and Sammon; McConnell, Sibley and Xu; Schroeder (13F chapter of a dissertation).

Not read, and not counted in the eight:
- Fu and Huang (buybacks): PDF request returned HTML or HTTP 403. Only a search-tool summary of the abstract and a CXO Advisory summary of the earlier working paper were seen.
- Peyer and Vermaelen: PDF download came back empty.
- Subrahmanyam (2025): seen only through a UCLA Anderson brief.
- Ozlen and Batumoglu (2026): seen only through a personal Substack, by search snippet.
- Ball and Brown, Lakonishok and Lee, Cusatis et al.: not opened.
- Barber et al. (2013): a draft was keyword-scanned only, and no figures from it are used here.

## For each paper: citation, test, sample, net-of-cost result, turnover, out-of-sample status, and cost issues for USD 2,500

### Takeaway
No paper read reports a net-of-cost result; all eight are gross or silent on costs, so every effect below still needs its own costed backtest. Later-sample evidence shows PEAD, the US earnings announcement premium and the S&P 500 index effect fading to about zero outside microcaps, and buyback drift gone in the abstract-level evidence.

### Cited Findings

**1. PEAD decay. Martineau, "Rest in Peace Post-Earnings Announcement Drift," Critical Finance Review 11(3-4), 2022.** Full text read: [PDF](https://cfr.ivo-welch.info/published/papers/martineau2021rest.pdf)
- Tested: whether analyst earnings surprises predict buy-and-hold abnormal returns over days 2 to 60 (BHAR[2,60]), split at the NYSE 20th size percentile into "all-but-microcap" and microcap. Table 2 covers 1984-2019, with 25,474 all-but-microcap and 12,528 microcap announcements in 2016-2019 — [Martineau](https://cfr.ivo-welch.info/published/papers/martineau2021rest.pdf)
- Result: since 2006, surprises fail to positively predict 60-day post-announcement returns for all-but-microcap stocks, and since 2016 for microcaps. After 2005 the all-but-microcap relation is not statistically significant — [Martineau](https://cfr.ivo-welch.info/published/papers/martineau2021rest.pdf)
- No hidden pocket: PEAD is reported absent for the smallest all-but-microcap stocks and at shorter horizons — [Martineau](https://cfr.ivo-welch.info/published/papers/martineau2021rest.pdf)
- Announcement-day price response in 2016-2019 is six times (all-but-microcap) and three times (microcap) that of 1984-1990 — [Martineau](https://cfr.ivo-welch.info/published/papers/martineau2021rest.pdf)
- Net of cost: not reported. The design is return regressions, not a costed portfolio. Turnover: not applicable. The paper is itself the post-publication decay evidence.

**2. Earnings announcement premium decay. Heitz, Narayanamoorthy and Zekhnini, "The Disappearing Earnings Announcement Premium," working paper, June 24, 2020.** Full text read: [PDF](https://www.iimb.ac.in/ARC2020/Papers/The_Disappearing_Earnings_Announcement_Premium.pdf)
- Tested: weekly announcer minus non-announcer portfolios, 1994-2016, over 1,100 weeks — [Heitz et al.](https://www.iimb.ac.in/ARC2020/Papers/The_Disappearing_Earnings_Announcement_Premium.pdf)
- Result: the weekly premium is 43.4 bps equal-weight (32.6 bps value-weight) before the 2004 8-K regulation, and 0.3 bps (8.7 bps) after, not statistically different from zero — [Heitz et al.](https://www.iimb.ac.in/ARC2020/Papers/The_Disappearing_Earnings_Announcement_Premium.pdf)
- The authors argue the premium moved to 8-K filing periods, reporting an additional 32 bps excess return with 8-K filings. They report the premium survives post-2004 in the nine non-US markets, for example 88 bps in the UK announcement week — [Heitz et al.](https://www.iimb.ac.in/ARC2020/Papers/The_Disappearing_Earnings_Announcement_Premium.pdf)
- Net of cost: not reported in the passages read. Weekly formation implies very high turnover.

**3. Earnings announcement premium. Frazzini and Lamont, "The Earnings Announcement Premium and Trading Volume," NBER WP 13090, May 2007.** Full text read: [PDF](https://www.nber.org/system/files/working_papers/w13090/w13090.pdf)
- Tested: monthly value-weight portfolios of expected announcers, 1973-2004, predicting the month from last year's announcement month. Accuracy is 80% overall and 93% for firms with four announcements in the prior year — [Frazzini and Lamont](https://www.nber.org/system/files/working_papers/w13090/w13090.pdf)
- Result: long-short earns 61 bps per month (about 7% per year, t above 5). Subperiods range from 40 to 92 bps per month. Annual Sharpe is 0.94 against a best of 0.70 among value, size, momentum and the market — [Frazzini and Lamont](https://www.nber.org/system/files/working_papers/w13090/w13090.pdf)
- Size: large caps earn a large, significant premium, and it is not concentrated in small stocks — [Frazzini and Lamont](https://www.nber.org/system/files/working_papers/w13090/w13090.pdf)
- Timing: about 25 bps in the 10 trading days before, 21 bps in the 3-day window, and 30 bps after — [Frazzini and Lamont](https://www.nber.org/system/files/working_papers/w13090/w13090.pdf)
- Net of cost: gross only. The authors describe the comparison as "before considering trading costs" and say frequent trading incurs substantial costs — [Frazzini and Lamont](https://www.nber.org/system/files/working_papers/w13090/w13090.pdf)
- Turnover: each firm is in the long leg four months a year and the short leg eight — [Frazzini and Lamont](https://www.nber.org/system/files/working_papers/w13090/w13090.pdf)
- Post-publication: see Heitz et al., which finds no US premium after 2004. That covers the last months of this sample and everything since.

**4. Insider purchases. Cohen, Malloy and Pomorski, "Decoding Inside Information," NBER WP 16454, October 2010.** Full text read: [PDF](https://www.nber.org/system/files/working_papers/w16454/w16454.pdf)
- Tested: Form 4 open-market trades, 1986-2007, excluding option exercises and private transactions. A routine trader traded in the same calendar month for at least three consecutive years; everyone else with sufficient history is opportunistic. Classification is fixed at the start of each year — [Cohen et al.](https://www.nber.org/system/files/working_papers/w16454/w16454.pdf)
- Result: opportunistic buys minus opportunistic sells earn a five-factor alpha of 82 bps per month value-weight (9.8% annualized, t=2.15) and 180 bps equal-weight (t=6.07). Routine buys minus routine sells earn -20 bps value-weight (t=-0.57) and 43 bps equal-weight (t=1.73) — [Cohen et al.](https://www.nber.org/system/files/working_papers/w16454/w16454.pdf)
- Mix: 64% of purchases are routine and 36% opportunistic — [Cohen et al.](https://www.nber.org/system/files/working_papers/w16454/w16454.pdf)
- Holding: portfolios are held for the month following the trade and rebalanced monthly. Median reporting lag is 3 days. Shifting returns to the 11th of the next month gives virtually identical results. Event-time returns rise for six months, then level off without reversal — [Cohen et al.](https://www.nber.org/system/files/working_papers/w16454/w16454.pdf)
- Size: the sample has fewer microcaps and about twice the share of top-decile stocks versus CRSP, and the authors say predictability is not limited to smaller firms — [Cohen et al.](https://www.nber.org/system/files/working_papers/w16454/w16454.pdf)
- Net of cost: no costed result found in the passages read. Evidence ends in 2007; no post-publication test was read.

**5. Insider purchases, timing of the return. NBER WP 6913 (Jeng, Metrick and Zeckhauser).** Text read from page 3 onward; pages 1-2 extracted blank, so the title page was not seen: [PDF](https://www.nber.org/system/files/working_papers/w6913/w6913.pdf)
- Tested: value-weighted portfolios of all reported insider trades, 1975-1996, with returns measured from the trade day — [Jeng et al.](https://www.nber.org/system/files/working_papers/w6913/w6913.pdf)
- Result: the purchase portfolio beats the market by about 7.4% per year raw, and abnormal performance is 37 to 47 bps per month. Sales show no comparable profit — [Jeng et al.](https://www.nber.org/system/files/working_papers/w6913/w6913.pdf)
- Timing: about one-sixth of abnormal return accrues in the first five days after the trade, one-third in the first month, three-quarters in six months — [Jeng et al.](https://www.nber.org/system/files/working_papers/w6913/w6913.pdf)
- Size: only 22.0% of the purchase portfolio's value comes from the largest-third stocks, against 83.1% of market value — [Jeng et al.](https://www.nber.org/system/files/working_papers/w6913/w6913.pdf)
- Net of cost: not reported in the passages read. Measured from the trade date, so it overstates what a filing-based follower can earn.

**6. Index reconstitution. Greenwood and Sammon, "The Disappearing Index Effect," NBER WP 30748, December 2022.** Full text read: [PDF](https://www.nber.org/system/files/working_papers/w30748/w30748.pdf)
- Tested: S&P 500 changes 1980-2020, market-adjusted return from the last trading day before announcement to the first trading day after implementation. The average gap to the effective date is 4.8 days for additions and 5.8 for deletions — [Greenwood and Sammon](https://www.nber.org/system/files/working_papers/w30748/w30748.pdf)
- Additions by decade: 3.4% (1980s), 7.6% (1990s), 5.2% (2000s), 0.8% (2010s, statistically indistinguishable from zero) — [Greenwood and Sammon](https://www.nber.org/system/files/working_papers/w30748/w30748.pdf)
- Deletions by decade: -4.6%, -16.6%, -12.3%, -0.6% (2010-2020, not distinguishable from zero) — [Greenwood and Sammon](https://www.nber.org/system/files/working_papers/w30748/w30748.pdf)
- Composition: migrations from the S&P MidCap rose from about 40% to over 80% of additions. In the late 2010s direct additions returned 2.2% and migrations -2.3% — [Greenwood and Sammon](https://www.nber.org/system/files/working_papers/w30748/w30748.pdf)
- Costs: value-weighted spreads fell about 10x from the early 1990s to the late 2010s, but the authors say this does not fully explain the decline. No strategy net-of-cost result is given — [Greenwood and Sammon](https://www.nber.org/system/files/working_papers/w30748/w30748.pdf)

**7. Spin-offs. McConnell, Sibley and Xu, "The Stock Price Performance of Spin-Off Subsidiaries, Their Parents, and the Spin-Off ETF, 2001-2013," Journal of Portfolio Management, Fall 2015.** Full text read: [PDF](https://business.purdue.edu/faculty/mcconnell/publications/The%20Stock%20Price...2013%20jpm.2015.42.1.143.pdf)
- Tested: 146 spin-offs in 2001-2012, 36-month returns, against size and book-to-market benchmarks. It is an out-of-sample continuation of a 1965-2000 study — [McConnell et al.](https://business.purdue.edu/faculty/mcconnell/publications/The%20Stock%20Price...2013%20jpm.2015.42.1.143.pdf)
- Subsidiaries, first 22 months: 1.26% per month raw (31.6% cumulative), 0.72% per month excess (17.1% cumulative) — [McConnell et al.](https://business.purdue.edu/faculty/mcconnell/publications/The%20Stock%20Price...2013%20jpm.2015.42.1.143.pdf)
- Parents, first 15 months: 0.85% per month raw, 0.27% per month excess (3.7% cumulative). In the earlier 1965-2000 sample, parents' excess return was largely due to one extreme outlier — [McConnell et al.](https://business.purdue.edu/faculty/mcconnell/publications/The%20Stock%20Price...2013%20jpm.2015.42.1.143.pdf)
- All subsidiaries held 22 months, equal-weight: 22.2% annual versus 17.4% for the spin-off ETF (CSD). The authors say prior performance "would have worked well, though perhaps by luck rather than skill" — [McConnell et al.](https://business.purdue.edu/faculty/mcconnell/publications/The%20Stock%20Price...2013%20jpm.2015.42.1.143.pdf)
- Net of cost: no cost analysis found in the passages read. Turnover is very low.

**8. 13F cloning. Schroeder, "Outperforming the Market: Portfolio Strategy Cloning from SEC 13F Filings," chapter 4 of the dissertation "From Regulatory Data to Quantitative Investment Signals in Equity Markets," TU Dortmund, November 2025.** Chapter passages read: [PDF](https://eldorado.tu-dortmund.de/bitstreams/b994312d-d582-438f-b97a-ccedfb0852cd/download). CXO Advisory lists the working paper as Schroeder and Posch, March 2024, SSRN 4767576 — [CXO](https://www.cxoadvisory.com/miscellaneous/form-13f-clone-portfolio-performance/); the chapter names Schroeder alone. SSRN returned 403.
- Tested: over 150,000 portfolios, 2013-2023, cloned portfolios rebalanced on the disclosure date, comparing clones with source funds on twelve metrics, 3,643 funds in the delta tables — [Schroeder](https://eldorado.tu-dortmund.de/bitstreams/b994312d-d582-438f-b97a-ccedfb0852cd/download)
- Result: top-quartile clones exceed the S&P 500 by 24.3% annualized risk-adjusted. Clones show higher volatility (median +3%) and larger tracking error — [Schroeder](https://eldorado.tu-dortmund.de/bitstreams/b994312d-d582-438f-b97a-ccedfb0852cd/download)
- Net of cost: explicitly none. The analysis does not incorporate trading costs or liquidity, and trades the position delta in a single day at the closing price — [Schroeder](https://eldorado.tu-dortmund.de/bitstreams/b994312d-d582-438f-b97a-ccedfb0852cd/download)
- Conflict flag: the author's listed email is at sec-api.io, the vendor that supplied the holdings data — [Schroeder](https://eldorado.tu-dortmund.de/bitstreams/b994312d-d582-438f-b97a-ccedfb0852cd/download)

**Buybacks, abstract-level only (PDF could not be read). Fu and Huang, "The Persistence of Long-Run Abnormal Returns Following Stock Repurchases and Offerings," Management Science 62(4), 2016, 964-984.**
- Per a search summary of the abstract: 14,309 open-market repurchases in 1984-2012. Long-run abnormal returns are significantly positive for events through 2002 and disappear for 2003-2012 events — [EconPapers](https://econpapers.repec.org/RePEc:inm:ormnsc:v:62:y:2016:i:4:p:964-984)
- The earlier working paper reports about 10% gross abnormal return over three years through 2002, gone in 2003-2008, with institutional ownership above 50% at 34% of repurchasers before 2003 and 70% after — [CXO Advisory](https://www.cxoadvisory.com/buybacks-secondaries/extinction-of-the-buybacksecondary-offering-effect)

### Inferences
- Commission arithmetic for a USD 2,460 account: with five equal positions of about USD 492, a USD 0.70 round trip is 14.2 bps per full portfolio turn (28.4 bps at doubled costs). With three positions of USD 820 it is 8.5 bps. Monthly full rotation at five positions costs about 1.7% per year in commission before spreads.
- That arithmetic makes the minimum commission a small issue for multi-month holds and a large one for weekly rotation. Heitz's post-2004 weekly premium of 0.3 to 8.7 bps is below a single round trip.
- Capacity is not the constraint at this size. Costs are the minimum commission plus spread, and spread is highest where the insider and PEAD effects are historically strongest.

### Gaps
- Long-leg-only alpha for opportunistic buys (Cohen et al., Table IV) was not extracted; only long-short figures are recorded.
- No net-of-cost replication of any of these effects was found for post-2010 large or mid caps.
- Peyer and Vermaelen, Lakonishok and Lee, and Cusatis et al. were not read, so nothing is claimed from them.

## Which effects persist after 2010 in large or mid caps rather than only microcaps?

### Takeaway
None is demonstrated net of costs after 2010 in large or mid caps. PEAD, the US announcement premium, the S&P 500 effect and buyback drift all show decay, leaving insider purchases and spin-offs as untested or thinly tested candidates.

### Cited Findings
- PEAD: not significant for all-but-microcaps after 2005, and for microcaps since 2016 — [Martineau](https://cfr.ivo-welch.info/published/papers/martineau2021rest.pdf)
- PEAD is contested, as read only through a secondary brief. Subrahmanyam (2025, SSRN 5930255), February 2001 to December 2024, reports an earnings drift factor t-statistic of 2.18 with all stocks and 1.43 excluding microcaps, arguing that microcap inclusion explains the disagreement. The brief also describes Hirshleifer, Peng and Wang (2025) with a drift t-statistic of about 14 and no microcap filter, and Dickerson et al. as finding that earnings drift remains a major factor — [UCLA Anderson Review](https://anderson-review.ucla.edu/is-post-earnings-announcement-drift-a-thing-again)
- That brief says the factor in question is computed from abnormal returns around announcements rather than accounting surprises, so it is not the same signal Martineau tests — [UCLA Anderson Review](https://anderson-review.ucla.edu/is-post-earnings-announcement-drift-a-thing-again)
- Announcement premium: premium of 0.3 bps (equal-weight) and 8.7 bps (value-weight) per week after 2004, not significant — [Heitz et al.](https://www.iimb.ac.in/ARC2020/Papers/The_Disappearing_Earnings_Announcement_Premium.pdf)
- S&P 500 additions 0.8% and deletions -0.6% in the 2010s, both indistinguishable from zero — [Greenwood and Sammon](https://www.nber.org/system/files/working_papers/w30748/w30748.pdf)
- Buybacks: effect gone for 2003-2012 events, per abstract-level summary — [EconPapers](https://econpapers.repec.org/RePEc:inm:ormnsc:v:62:y:2016:i:4:p:964-984)
- Spin-offs: subsidiaries' excess return of 0.72% per month for 2001-2012 events, equal-weighted across a set not restricted to large caps — [McConnell et al.](https://business.purdue.edu/faculty/mcconnell/publications/The%20Stock%20Price...2013%20jpm.2015.42.1.143.pdf)
- 13F: 2013-2023 clones show gross outperformance in the top quartile, without costs — [Schroeder](https://eldorado.tu-dortmund.de/bitstreams/b994312d-d582-438f-b97a-ccedfb0852cd/download)
- Insider filings, unverified: a personal blog attributes to a January 2026 SSRN paper by Ozlen and Batumoglu the estimate that 70-80% of insider alpha lies between the trade and public filing — [Substack](https://tommijohnsen.substack.com/p/when-the-insiders-and-the-news-disagree). The paper itself was not found, so treat this as a lead only.

### Inferences
- If only large and mid caps are tradable, drop PEAD, the announcement premium, index events and buybacks as primary strategies unless a costed test on recent data overturns the published decay.
- Insider purchases are the best-supported candidate to test, but the read evidence stops in 2007 and the reported tilt toward small stocks means a mid-cap filter may remove much of the effect.
- The 24.3% clone figure should not be used as an expected return. "Top quartile" appears to describe the ex-post cross-section of funds, which a live investor could not have selected in advance. I did not confirm this from the chapter's method section.

### Gaps
- No paper was found on spin-off long-run returns for events after 2012.
- No read paper tests insider purchases after 2007, in any size group.
- The Subrahmanyam, Hirshleifer et al. and Dickerson et al. results were seen only through the UCLA brief.

## Which can be implemented long-only with a few positions, using free data and point-in-time filing timestamps?

### Takeaway
Insider purchases (Form 4), and to a lesser degree top-holding 13F cloning, fit long-only, few positions and free EDGAR data with exact timestamps. Spin-offs fit on cost but are few in number and hold longer than the stated horizon, while PEAD lacks a free point-in-time consensus estimate.

### Cited Findings
- Form 4 deadline is 10 p.m. Eastern on the second business day after the transaction, per issuer policy documents surfaced in search — [S&P Global policy exhibit](https://www.sec.gov/Archives/edgar/data/64040/000006404025000052/ex193tofy2024formxdirector.htm)
- Forms 3, 4 and 5 filed between 5:30 and 10:00 p.m. Eastern receive that same business day's filing date — [SEC Release 33-8230](https://www.sec.gov/files/rules/final/33-8230.htm)
- Form 13F is due within 45 days after the calendar quarter-end — [SEC 13F FAQ](https://www.sec.gov/files/divisions/investment/13ffaq.htm)
- 13F holdings omit short positions and can be withheld under confidential treatment for three months to a year. This came from a search-tool summary that did not attribute it to a single source, so confirm it against the [SEC 13F FAQ](https://www.sec.gov/files/divisions/investment/13ffaq.htm)
- Form 8-K reports are generally due within four business days of the event. Item 2.02 information is furnished rather than filed — [Form 8-K](https://www.sec.gov/files/form8-k.pdf)
- Cohen et al.'s classification needs three prior years of an insider's trades, which is why only about one-third of the transaction sample is usable — [Cohen et al.](https://www.nber.org/system/files/working_papers/w16454/w16454.pdf)
- Announcement months can be predicted without look-ahead from the prior year's month — [Frazzini and Lamont](https://www.nber.org/system/files/working_papers/w13090/w13090.pdf)
- Spin-offs numbered 146 over 2001-2012, about 12 a year — [McConnell et al.](https://business.purdue.edu/faculty/mcconnell/publications/The%20Stock%20Price...2013%20jpm.2015.42.1.143.pdf)

### Inferences
| Effect | Long-only, few positions | Free data feasible | Recent evidence | Verdict for testing |
|---|---|---|---|---|
| Opportunistic insider buys | Yes | Yes, Form 4 XML | Read evidence ends 2007 | First priority |
| 13F top-holding cloning | Yes, if concentrated | Yes, 13F-HR | Gross only, 2013-2023 | Second, with skepticism |
| Spin-off subsidiaries | Yes, but lumpy | Partly, event list is manual | Events through 2012 | Third; 22-month hold exceeds the horizon |
| Announcement premium | Yes, high turnover | Yes, 8-K history | Gone after 2004 in US | Run as a negative control |
| PEAD | Yes | Consensus not free | Gone outside microcaps | Run as a negative control |
| Buybacks | Yes | Poor event identification | Gone after 2002 (abstract) | Reject |
| Index additions | Yes | Not on EDGAR | About zero in 2010s | Reject |

- A sparse strategy like spin-offs is unlikely to meet the "no negative year" and weekly-cluster bootstrap requirements, simply because few independent weekly clusters exist.
- None of the papers applies the evaluation bar (doubled costs, weekly-cluster bootstrap, no negative year, beating equal-weight, opposite side failing). All of it must be run in-house.
- Free daily price sources commonly omit delisted tickers, which would bias event studies upward. Schroeder specifically sourced survivorship-free prices from a paid vendor — [Schroeder](https://eldorado.tu-dortmund.de/bitstreams/b994312d-d582-438f-b97a-ccedfb0852cd/download)

### Gaps
- SEC pages were seen only as search summaries; sec.gov returned errors to direct fetching in this session. Confirm deadlines against the live rules.
- No free, point-in-time source for analyst consensus was identified.
- The availability of delisted-name history from free price sources was not researched.

## Exact rule definitions, with event timestamp and entry timing that avoid look-ahead

### Takeaway
Use the EDGAR acceptance timestamp as the event time and enter at the next regular-session open strictly after it. The rules below are my proposed specifications adapted from the papers, not rules any paper tested in this exact form.

### Cited Findings
- A filing dated day D may have been submitted as late as 10 p.m. Eastern on D, so D's close was not necessarily tradable — [SEC Release 33-8230](https://www.sec.gov/files/rules/final/33-8230.htm); [EDGAR Filer Manual](https://www.sec.gov/files/edgar/filermanual/efmvol2-c2.pdf)
- Schroeder trades at the closing price on the filing date, and CXO describes clones as updated at the market close on filing dates — [Schroeder](https://eldorado.tu-dortmund.de/bitstreams/b994312d-d582-438f-b97a-ccedfb0852cd/download); [CXO](https://www.cxoadvisory.com/miscellaneous/form-13f-clone-portfolio-performance/)
- Jeng et al. measure from the trade date, while Cohen et al. rely on a median 3-day reporting lag — [Jeng et al.](https://www.nber.org/system/files/working_papers/w6913/w6913.pdf); [Cohen et al.](https://www.nber.org/system/files/working_papers/w16454/w16454.pdf)
- Martineau scales surprises by the price five trading days before the announcement and measures drift from day 2 to day 60 — [Martineau](https://cfr.ivo-welch.info/published/papers/martineau2021rest.pdf)
- Most of the historical announcement premium accrued outside the 3-day window, including a 10-day run-up — [Frazzini and Lamont](https://www.nber.org/system/files/working_papers/w13090/w13090.pdf)

### Inferences

**Common rules for all strategies**
- Event time T is the EDGAR acceptance timestamp in Eastern Time. If only the filing date is available, assume acceptance after that day's close.
- Entry is at the open of the first NYSE trading day strictly after the calendar date of T. This is deliberately conservative and is safe on daily bars. A more aggressive variant allows same-day open only if T is before about 09:00 ET, and should be tested separately.
- Universe at the prior close: US common stock, price at least USD 5, market cap above a pre-set floor (use the NYSE 20th percentile or a fixed USD 500 million), and a pre-set dollar-volume floor. Use only data known by that close.
- Portfolio: maximum 3 to 5 equal-weight positions. If full, skip new signals in first-come order, with ties broken by a pre-set deterministic key. Do not rank using data unavailable at T.
- Costs: USD 0.35 per order minimum plus a spread and slippage assumption by size bucket, then the same at double.
- Include delisting returns; a position that delists is closed at the last available value less a haircut.

**Rule A: opportunistic insider purchases (Form 4). Priority one.**
1. Event: an original Form 4 (ignore 4/A for signal generation), with a non-derivative transaction coded P, acquired, by an officer or director. Test 10% holders separately.
2. Classify each insider each January 1 using only filings accepted before that date. Routine means a trade in the same calendar month in each of at least the three preceding years. Opportunistic means at least three years of history and not routine, following Cohen et al.
3. Signal: at least one opportunistic P purchase, with a minimum dollar value fixed in advance. Exclude trades flagged as made under a 10b5-1 plan where the form identifies this.
4. Entry at the common-rule open. Exit at the open after 126 trading days. Test 21 and 63 as pre-registered alternates, since returns were reported to accumulate for about six months.
5. Controls: routine buys and opportunistic sells should not pass as long signals. Compare against an equal-weight universe benchmark.

**Rule B: concentrated 13F new-position clone. Priority two.**
1. Event: acceptance of an original 13F-HR. Exclude options (put or call rows) and ETFs.
2. Manager selection uses only filings and returns available before the event, for example trailing clone performance through the prior quarter. Never select on full-sample performance.
3. Signal: the largest new or materially increased positions, up to the portfolio limit.
4. Entry at the common-rule open. Exit at the open after the same manager's next 13F-HR acceptance, if the position has been cut or the next filing supersedes it.

**Rule C: spin-off subsidiary. Priority three.**
1. Event: first regular-way trading day of the subsidiary. Event lists must be assembled from Form 10 and 8-K filings; do not trust a list built later without checking dates.
2. Entry at the open of the following trading day, to avoid when-issued and first-day auction prices. Exit at the open after about 22 months (462 trading days), the window in McConnell et al.
3. Note that this exceeds the requested days-to-months horizon. Test 6 and 12 months as well.

**Rule D: PEAD (negative control).**
1. Event: acceptance of an 8-K containing Item 2.02. Surprise must come only from data public at that time. XBRL figures arrive with the later 10-Q, so a seasonal random-walk surprise built from XBRL is timestamped at the 10-Q acceptance, not the earnings date, unless the 8-K exhibit is parsed.
2. Entry at the common-rule open. Exit at the open 60 trading days later. Top-surprise names only, long-only.
3. Expect failure outside microcaps, given Martineau.

**Rule E: announcement premium (negative control).**
1. Expected date equals the same company's prior-year equivalent 8-K Item 2.02 acceptance date plus 364 days. Never use the actual date for selection.
2. Entry at the open 10 trading days before the expected date. Exit at the open 3 trading days after the actual filing.
3. Expect failure in the US after 2004, given Heitz et al.

**Rejected, no specification recommended.** Index additions: the published 2010s total return is about zero and is measured from before the announcement, part of which cannot be captured after a public announcement. Buybacks: published drift is gone after 2002 at abstract level, and I would expect event identification from 8-K text to be unreliable because I found no dedicated item for repurchase authorizations.

### Gaps
- The EDGAR field names for acceptance time, and the 10b5-1 checkbox on Form 4, are from background knowledge and were not verified in this session. Confirm against EDGAR before coding.
- The USD 500 million floor, the 126-day hold variants and the 13F selection rule are design choices, not values validated by the papers read.
- Whether Rule A survives the evaluation bar is unknown, because no backtest was run for these notes.
