# Systematic ETF trend and tactical allocation for a USD 2,500 retail account

Scope note: no backtest was run for these notes. Every performance number below is what a source reports. Calculations of mine are in the Inferences subsections and are labelled.

## Paper-by-paper evidence: citation, test, sample, net result, turnover, out-of-sample status, small-account issues

### Takeaway
Seven of the eight target papers were read as full-text PDFs. Antonacci could not be read. Almost no source reports net results that apply to a small ETF account: the futures papers are gross or use institutional cost models, Faber excludes costs, and Keller is the only ETF-universe paper that deducts a cost (0.1% one-way). The strongest skeptical results are Huang et al. (time-series predictability is statistically weak) and Cederburg et al. (volatility management does not reliably help real-time investors).

### Cited Findings

**Reading status**

| # | Paper | What was actually read |
|---|---|---|
| 1 | Moskowitz, Ooi, Pedersen 2012 | Full PDF text extracted; targeted reading of definitions, results and a text search for costs |
| 2 | Hurst, Ooi, Pedersen 2017 | Full PDF read, including Exhibit 1 as a page image. Exhibits 2 and B1 not read numerically |
| 3 | Huang, Li, Wang, Zhou 2020 | Full PDF text extracted (published JFE version); targeted reading |
| 4 | Faber 2013 update | Full PDF read through page 59 of 70; Figure 16 read as a page image. Figures 13, 15 and 20 numeric tables are images and were not read |
| 5 | Keller 2022 (BAA) | Full PDF text read through the conclusion (page 12 of 14), from a third-party copy of the SSRN paper. SSRN itself returned HTTP 403 |
| 6 | Antonacci, dual momentum | **PDF could not be read.** SSRN returned 403, the INFLIBNET repository copy is marked Restricted Access, Alpha Architect returned 403. Only search snippets and secondary pages were seen |
| 7 | Moreira, Muir 2017 | Published PDF text extracted; targeted reading of construction and the cost table |
| 8 | Cederburg, O'Doherty, Wang, Yan 2020 | Published PDF text extracted; targeted reading |

**1. Moskowitz, T., Ooi, Y.H., Pedersen, L.H. (2012). "Time Series Momentum." Journal of Financial Economics 104(2), 228-250.** [PDF](https://pages.stern.nyu.edu/~lpederse/papers/TimeSeriesMomentum.pdf)
- Tested: 58 liquid futures and forwards across equity index, currency, commodity and bond markets; data January 1965 to December 2009, with main results reported January 1985 to December 2009. — [MOP](https://pages.stern.nyu.edu/~lpederse/papers/TimeSeriesMomentum.pdf)
- Main strategy: 12-month lookback, 1-month hold, position sized to 40% ex ante annualized volatility, averaged equally across instruments. The portfolio ran about 12% annualized volatility over 1985-2009. — [MOP](https://pages.stern.nyu.edu/~lpederse/papers/TimeSeriesMomentum.pdf)
- Result: Table 3 monthly alpha of 1.58% (t = 7.99) against MSCI World and Fama-French SMB, HML, UMD, and 1.09% (t = 5.40) against the Asness-Moskowitz-Pedersen factors. The paper describes a Sharpe ratio above one, roughly 2.5 times the equity market. — [MOP](https://pages.stern.nyu.edu/~lpederse/papers/TimeSeriesMomentum.pdf)
- Net of cost: **gross only.** Instrument Sharpe ratios are labelled gross, and a text search found no transaction-cost estimate or turnover figure for the strategy; the only "transaction costs" hits are a citation to Garleanu and Pedersen. — [MOP](https://pages.stern.nyu.edu/~lpederse/papers/TimeSeriesMomentum.pdf)
- Out-of-sample: the authors report a statistically significant return and a Sharpe of 1.1 for 1966-1985, which they call out-of-sample evidence. Nothing after 2009. — [MOP](https://pages.stern.nyu.edu/~lpederse/papers/TimeSeriesMomentum.pdf)

**2. Hurst, B., Ooi, Y.H., Pedersen, L.H. (2017). "A Century of Evidence on Trend-Following Investing." Journal of Portfolio Management 44(1).** [PDF](https://fairmodel.econ.yale.edu/ec439/hurst.pdf)
- Tested: 67 markets (29 commodities, 11 equity indices, 15 bonds, 12 currency pairs), January 1880 to December 2016; equal-weighted blend of 1-, 3- and 12-month signals, long/short, rebalanced monthly, scaled to 10% ex ante portfolio volatility. — [HOP](https://fairmodel.econ.yale.edu/ec439/hurst.pdf)
- Full-sample annualized excess return: 18.0% gross, 11.0% net of simulated transaction costs, 7.3% net of 2-and-20 fees; net Sharpe 0.76, volatility 9.7%, correlation to US equities -0.01. — [HOP Exhibit 1](https://fairmodel.econ.yale.edu/ec439/hurst.pdf)
- Costs are proprietary 2012 estimates by asset class, doubled for 1993-2002 and multiplied by six for 1880-1992. The authors say they are uncertain and exclude futures roll costs. — [HOP Appendix B](https://fairmodel.econ.yale.edu/ec439/hurst.pdf)
- Out-of-sample: the authors treat the pre-1985 decades as out-of-sample relative to MOP. Lagging the signal by one month still gave positive returns in most decades, but performance deteriorated, especially for shorter signals. — [HOP](https://fairmodel.econ.yale.edu/ec439/hurst.pdf)
- Drawdowns reached 25%. Performance was weakest when pairwise correlations across markets were high; average correlations rose meaningfully from late 2008 to mid-2014. — [HOP](https://fairmodel.econ.yale.edu/ec439/hurst.pdf)
- The paper notes the early-period strategy was not implementable as described, and that its main point is the existence of trends. — [HOP endnote 6](https://fairmodel.econ.yale.edu/ec439/hurst.pdf)

**3. Huang, D., Li, J., Wang, L., Zhou, G. (2020). "Time Series Momentum: Is It There?" Journal of Financial Economics 135(3), 774-794.** [PDF](https://down.aefweb.net/WorkingPapers/w717.pdf)
- Tested: 55 futures contracts built to match MOP, January 1985 to December 2015; strategy returns over 1986:01-2015:12. — [Huang et al.](https://down.aefweb.net/WorkingPapers/w717.pdf)
- Asset-by-asset: 47 of 55 assets have t < 1.65 at the 10% level, and only three deliver a significant out-of-sample R-squared at the 10% level. — [Huang et al.](https://down.aefweb.net/WorkingPapers/w717.pdf)
- Pooled regression: t = 4.34, below the 5% bootstrap critical values of 12.53 (wild) and 4.83 (pairs). — [Huang et al.](https://down.aefweb.net/WorkingPapers/w717.pdf)
- Trading: the benchmark TSH rule is long if the asset's cumulative return from the start of the sample is positive and short if negative. It needs no predictability. — [Huang et al.](https://down.aefweb.net/WorkingPapers/w717.pdf)
- Equal-weighted long-short mean: TSM 0.39, TSH 0.25; difference 0.14 (p = 0.19). Volatility-weighted: TSM 1.16, TSH 0.74; difference 0.42 (p = 0.07), alpha differences 0.32 (p = 0.08) and 0.10 (p = 0.56). Table units read as percent per month. — [Huang et al., long/short portfolio table, pp.788-789](https://down.aefweb.net/WorkingPapers/w717.pdf)
- Profit comes from the long leg. Equal-weighted TSM long leg 0.34 (t = 4.92), short leg -0.05 (t = -0.72); volatility-weighted long 1.02 (t = 7.66), short -0.14 (t = -1.07). — [Huang et al.](https://down.aefweb.net/WorkingPapers/w717.pdf)
- Subperiod: the authors test whether TSM existed before 2009 and then vanished, and reject that reading because real-data t-statistics are below simulated critical values for 1985-2009 too. — [Huang et al.](https://down.aefweb.net/WorkingPapers/w717.pdf)
- Net of cost: **gross only.** A text search found no transaction-cost or turnover analysis. — [Huang et al.](https://down.aefweb.net/WorkingPapers/w717.pdf)

**4. Faber, M. (2007, updated February 2013). "A Quantitative Approach to Tactical Asset Allocation." Journal of Wealth Management, Spring 2007; SSRN 962461.** [PDF](https://mebfaber.com/wp-content/uploads/2016/05/SSRN-id962461.pdf)
- Tested: 10-month SMA timing on the S&P 500 from 1901 to 2012, and a five-asset portfolio (S&P 500, MSCI EAFE, US 10-year, GSCI, NAREIT) from 1973 to 2012, on index total-return data from Global Financial Data. — [Faber](https://mebfaber.com/wp-content/uploads/2016/05/SSRN-id962461.pdf)
- Net of cost: **excluded.** "Taxes, commissions, and slippage are excluded." — [Faber p.22](https://mebfaber.com/wp-content/uploads/2016/05/SSRN-id962461.pdf)
- S&P timing 1901-2012: compound return 10.18% against 9.32% buy-and-hold, about 70% invested, under one round trip per year. Max drawdown 42.24% against 83.66%. — [Faber](https://mebfaber.com/wp-content/uploads/2016/05/SSRN-id962461.pdf)
- Turnover: three to four round trips per year for the portfolio, under one per asset class per year, "almost 70%" turnover. Average investment level about 70%. — [Faber pp.29, 35-36](https://mebfaber.com/wp-content/uploads/2016/05/SSRN-id962461.pdf)
- Out-of-sample by the author (after the original results through 2005): 2006-2012 GTAA return 6.01%, volatility 7.27%, Sharpe 0.61, max drawdown -9.42%, against equal-weight buy-and-hold 3.94%, 14.96%, 0.16, -46.00%. It outperformed in only three of the seven years. — [Faber Figure 16](https://mebfaber.com/wp-content/uploads/2016/05/SSRN-id962461.pdf)
- Parameter stability is claimed for SMA lengths of 3 to 12 months. — [Faber p.33 and FAQ](https://mebfaber.com/wp-content/uploads/2016/05/SSRN-id962461.pdf)
- Later extension, **secondary only**: a CXO Advisory summary of a paywalled Zarattini study reports 6.0% gross CAGR and Sharpe 0.68 for January 2006 to March 2025, against 11.7% and 0.81 for the original 1972-2005 period. No benchmark or net figure was visible. — [CXO Advisory](https://www.cxoadvisory.com/strategic-allocation/extension-of-a-quantitative-approach-to-tactical-asset-allocation)

**5. Keller, W.J. (July 18, 2022, v0.98). "Relative and Absolute Momentum in Times of Rising/Low Yields: Bold Asset Allocation (BAA)." SSRN 4166845.** [PDF copy](https://thechartist.com.au/wp-content/uploads/2024/12/5369/SSRN-id4166845.pdf)
- Tested: 17 ETFs extended with monthly total-return proxies, December 1970 to June 2022. Proxy construction is described in an earlier paper not read here. — [Keller](https://thechartist.com.au/wp-content/uploads/2024/12/5369/SSRN-id4166845.pdf)
- Net of cost: one-way transaction cost of 0.1% assumed for all strategies. — [Keller p.5](https://thechartist.com.au/wp-content/uploads/2024/12/5369/SSRN-id4166845.pdf)
- BAA-G4 (Top 1 offensive): full-sample CAGR 21.0%, max monthly drawdown 14.6%, Sharpe 1.21, one-sided annual turnover 523%, 6.6 trading months per year. BAA-G12 (Top 6): 14.6%, 8.7%, 1.19, 472%, 8.2. 60/40 benchmark 9.5% CAGR, 29.5% drawdown. — [Keller figs 3, 6](https://thechartist.com.au/wp-content/uploads/2024/12/5369/SSRN-id4166845.pdf)
- June 2012 to June 2022: BAA-G4 10.9%, BAA-G4/T2 9.2%, BAA-SPY 10.1%, BAA-G12 7.8%, BAA-G12/T3 6.6%, against 60/40 8.8% and SPY 12.6%. Drawdowns were smaller than SPY's 20.0%. — [Keller figs 3, 6, 9-12](https://thechartist.com.au/wp-content/uploads/2024/12/5369/SSRN-id4166845.pdf)
- Selection caution: the author labels December 1980 to June 2012 as "OS", but in-sample periods are the first and last decade of the data, and several choices were picked as in-sample optimal (filter pairing, adding DBC, QQQ for SPY). — [Keller notes 6, 8, 10](https://thechartist.com.au/wp-content/uploads/2024/12/5369/SSRN-id4166845.pdf)
- Filter fragility: for BAA-G4, return/risk score K falls to 0 when the canary filter is SMA(9), SMA(12) or RET(12), and K is defined as 0 when drawdown exceeds 25%. — [Keller fig 14 and note 9](https://thechartist.com.au/wp-content/uploads/2024/12/5369/SSRN-id4166845.pdf)
- Related DAA rule (blog by the DAA co-developer, not the paper): canary VWO and BND, 100% in the best of SHY, IEF, LQD when both are negative; the page states results ignore trading costs, slippage and taxes. — [TrendXplorer](https://indexswingtrader.blogspot.com/2018/07/announcing-defensive-asset-allocation.html)

**6. Antonacci, G. "Risk Premia Harvesting Through Dual Momentum." Journal of Management & Entrepreneurship 11(1), 2017, 27-55 per a repository listing. PDF NOT READ; only secondary material seen.**
- The repository item is marked Restricted Access, so no results from the paper are reported here. — [INFLIBNET record](https://gnanaganga.inflibnet.ac.in:8443/jspui/handle/123456789/14295?mode=full)
- The 2012 precursor used monthly data from January 1974 to December 2011, according to a secondary summary seen only as a search snippet. — [Alpha Architect](https://alphaarchitect.com/2012/07/risk-premia-harvesting-through-momentum/)
- Post-publication, weak source: a commercial trading-education blog reports about 7.2% CAGR against 6.4% for SPY over January 2008 to December 2013, then annualized returns about a third of that pre-publication level over January 2014 to March 2023, with max drawdown matching SPY. Seen as a search snippet, apparently gross, promotional source. — [Price Action Lab](https://priceactionlab.com/Blog/?p=68274)

**7. Moreira, A., Muir, T. (2017). "Volatility-Managed Portfolios." Journal of Finance, August 2017, DOI 10.1111/jofi.12513.** [PDF](https://amoreira2.github.io/alan-moreira.github.io/VolPortfolios_published.pdf)
- Tested: factor portfolios scaled by the inverse of the prior month's realized variance, using data from 1926 to 2015. Market alpha 4.86% annualized with beta about 0.6. — [Moreira, Muir](https://amoreira2.github.io/alan-moreira.github.io/VolPortfolios_published.pdf)
- Net of cost (market factor only, Table IV), alpha after 1, 10 and 14 bps and break-even cost: realized variance weight 4.77%, 3.98%, 3.63%, 56 bps (mean absolute weight change 0.73); realized volatility 3.80%, 3.39%, 3.21%, 84 bps (0.38); no-leverage cap at 1: 2.10%, 1.93%, 1.85%, 110 bps (0.16); 1.5 cap: 3.08%, 2.91%, 2.83%, 161 bps (0.16). — [Moreira, Muir Table IV](https://amoreira2.github.io/alan-moreira.github.io/VolPortfolios_published.pdf)
- The capped-at-1 version has expected return 5.61% against 9.47% for the baseline variance-weighted strategy. The authors do not report costs for other factors, saying they lack good measures. — [Moreira, Muir Table IV](https://amoreira2.github.io/alan-moreira.github.io/VolPortfolios_published.pdf)
- The scaling constant c is set on the full sample; the authors note it does not affect the Sharpe ratio. — [Moreira, Muir](https://amoreira2.github.io/alan-moreira.github.io/VolPortfolios_published.pdf)

**8. Cederburg, S., O'Doherty, M.S., Wang, F., Yan, X. (2020). "On the Performance of Volatility-Managed Portfolios." Journal of Financial Economics 138(1), 95-117.** [PDF](https://www.lehigh.edu/~xuy219/research/COWY.pdf)
- Tested: 103 equity strategies (9 factors, 94 anomalies). Factor samples start between 1926 and 1967 and end in December 2016. — [Cederburg et al.](https://www.lehigh.edu/~xuy219/research/COWY.pdf)
- Direct comparison: the managed version has the higher Sharpe in 53 of 103 cases, the original in 50; only eight managed wins are significant, concentrated in momentum strategies. — [Cederburg et al.](https://www.lehigh.edu/~xuy219/research/COWY.pdf)
- Spanning regressions replicate: 77 of 103 positive alphas, 23 significantly positive, 3 significantly negative. — [Cederburg et al.](https://www.lehigh.edu/~xuy219/research/COWY.pdf)
- Real-time: combining managed and original portfolios earns lower certainty-equivalent return in 72 of 103 cases. For the market, Sharpe is 0.42 against 0.46 for the unmanaged portfolio. Gains appear for MOM, ROE and BAB. The cause identified is structural instability in the spanning regressions. — [Cederburg et al.](https://www.lehigh.edu/~xuy219/research/COWY.pdf)
- Net of cost: **gross.** A text search found no transaction-cost analysis. — [Cederburg et al.](https://www.lehigh.edu/~xuy219/research/COWY.pdf)

### Inferences
- My calculation: one USD 0.35 order is 1.42 bps of a USD 2,460 account. It is 7.1 bps of a USD 492 position (a 20% sleeve).
- Capacity is not a constraint at this size for liquid ETFs. The binding cost is commission per order, so order count matters more than turnover percentage.
- My calculation, commissions only: Faber GTAA at 3-4 round trips is 6-8 orders per year, about USD 2.10-2.80 or 9-11 bps of NAV. Rebalancing all five sleeves monthly would be 60 orders, USD 21 or 85 bps, which should be avoided by trading only on signal changes.
- My calculation, BAA-G4: at most about 26 orders per year (6.6 trading months, up to 4 legs), about USD 9.24 or 38 bps, plus spread. Keller's own cost model at 0.1% one-way on 523% turnover implies about 52 bps a year, and doubled costs about 105 bps.
- The Sharpe row in Faber's Figure 16 is labelled 5.41%, but the reported Sharpe ratios imply a risk-free rate near 1.6%. The label looks wrong for that period, so recompute rather than cite those Sharpe ratios.

### Gaps
- Antonacci's dual momentum results could not be verified from the paper. Do not cite performance numbers for it from these notes.
- HOP Exhibit B1 (assumed cost levels by asset class) was not read numerically.
- No source measured ETF spreads for this account size, so spread cost remains an assumption for the backtest.

## Does trend following or tactical allocation add net value after 2010 and after publication?

### Takeaway
The evidence does not show reliable net value over buy-and-hold after 2010. Trend's reported returns fell by decade while staying positive through 2016, and the main measurable benefit in the sources is lower drawdown rather than higher return. Treat any edge as a risk-reduction hypothesis to test, not an established return premium.

### Cited Findings
- Decade decline, HOP excess returns (gross, net of cost, net of cost and fees, net Sharpe): 1990-99 16.8%, 12.3%, 8.3%, 0.98; 2000-09 11.6%, 9.9%, 6.3%, 0.61; 2010-16 7.6%, 6.2%, 3.3%, 0.41. — [HOP Exhibit 1](https://fairmodel.econ.yale.edu/ec439/hurst.pdf)
- 2010-16 correlation to US equities was -0.15, so the strategy stayed diversifying while returns shrank. — [HOP Exhibit 1](https://fairmodel.econ.yale.edu/ec439/hurst.pdf)
- Faber's five-asset model beat equal-weight buy-and-hold in 2006-2012 by about two points a year while outperforming in only three of seven years. — [Faber Figure 16](https://mebfaber.com/wp-content/uploads/2016/05/SSRN-id962461.pdf)
- For June 2012 to June 2022, no BAA variant beat SPY's 12.6% CAGR; BAA-G4, BAA-G4/T2 and BAA-SPY beat 60/40's 8.8%, while BAA-G12 and BAA-G12/T3 did not. BAA was published in July 2022, so this period is in-sample. — [Keller](https://thechartist.com.au/wp-content/uploads/2024/12/5369/SSRN-id4166845.pdf)
- Keller's footnote reports SPY switching to IEF on SMA10 at 9.6% CAGR, 20.3% drawdown and Sharpe 0.80 for June 2002 to June 2022, against SPY 8.9%, 50.8% and 0.53. — [Keller note 11, fig 12](https://thechartist.com.au/wp-content/uploads/2024/12/5369/SSRN-id4166845.pdf)
- Search-result summaries of AQR material say trend significantly underperformed in the 2010s, that the SG Trend Index beat 60/40 since 2000, and that it returned 36% through September 2022 against -20% for global 60/40. This is the manager's own framing and the white paper was not opened. — [AQR](https://www.aqr.com/insights/research/white-papers/trend-following-why-now-a-macro-perspective)
- Huang et al. find TSM statistically weak and close to a rule needing no predictability. — [Huang et al.](https://down.aefweb.net/WorkingPapers/w717.pdf)
- Volatility management: the original in-sample alpha does not carry over to real-time investors in most cases, including the market factor. — [Cederburg et al.](https://www.lehigh.edu/~xuy219/research/COWY.pdf)

### Inferences
- HOP's 2010-16 figures are the cleanest partly post-sample number for diversified futures trend: positive after costs, weak after fees. A long-only, unlevered, few-ETF version is a materially different product, so these numbers do not transfer.
- Faber's post-2005 outperformance is dominated by avoiding 2008. A backtest starting after 2009 would exclude that episode and is the harder and more relevant test.
- Huang's result implies the correct benchmark is equal-weight buy-and-hold of the same ETFs. Beating cash or a single asset would not show timing skill.
- Huang's finding that short legs do not earn returns supports the long-only constraint, and suggests much of a long-only result may be asset drift. The "opposite side should not pass" test should be long when below trend.
- The "no negative year" bar is likely to fail for most long-biased variants, since Faber's model itself had a down year and HOP's drawdowns reached 25%. This is untested on the ETF data.

### Gaps
- I found no peer-reviewed, net-of-cost, post-publication evaluation of dual momentum, Faber GTAA or the Keller family on tradable ETFs.
- I did not find a reliable annualized SG Trend figure for the 2010s or after 2016.
- Post-2016 academic evidence on time-series momentum was not read, because of the 8-paper limit.

## Which rule variants need only a handful of ETFs and monthly rebalancing?

### Takeaway
Dual momentum (one ETF held at a time), Faber GTAA-5 (five sleeves plus cash) and BAA-SPY or BAA-G4 are the practical candidates. All are monthly. Faber has the lowest documented trading; BAA has the highest turnover and the most in-sample selection.

### Cited Findings
- Faber GTAA-5: five assets plus T-bills, month-end signal only, three to four round trips a year. — [Faber](https://mebfaber.com/wp-content/uploads/2016/05/SSRN-id962461.pdf)
- BAA-SPY: one offensive asset, four canaries, seven defensive assets, 473% turnover, 6.0 trading months a year; full-sample 11.4% CAGR, 16.2% drawdown against SPY 10.4%, 50.8%. — [Keller fig 11](https://thechartist.com.au/wp-content/uploads/2024/12/5369/SSRN-id4166845.pdf)
- BAA-G4: holds one offensive ETF or three defensive ETFs. — [Keller fig 6](https://thechartist.com.au/wp-content/uploads/2024/12/5369/SSRN-id4166845.pdf)
- Dual momentum holds one ETF at a time, using S&P 500, ex-US equity, an aggregate bond ETF, and T-bills as the hurdle. — [Severian guest post](https://hedgefundalpha.com/strategies/is-gary-antonaccis-global-equity-momentum-strategy-robust/); [Quantopian forum archive](https://quantopian-archive.netlify.app/forum/threads/dual-momentum-investing-strategy-according-to-dr-gary-antonacci)
- Antonacci's related Parity Portfolio variant uses five asset classes at 20% each, held only if 12-month momentum exceeds a T-bill, otherwise in aggregate bonds, rebalanced monthly (third-party description). — [TuringTrader](https://turingtrader-com.web.app/portfolios/antonacci-dual-momentum)
- Moreira and Muir's capped version needs one ETF plus cash, with a mean absolute monthly weight change of 0.16. — [Moreira, Muir Table IV](https://amoreira2.github.io/alan-moreira.github.io/VolPortfolios_published.pdf)
- HOP's three-signal design needs many markets and shorting, and is not reproduced by a few long-only ETFs. — [HOP](https://fairmodel.econ.yale.edu/ec439/hurst.pdf)

### Inferences
- Priority for a pre-registered test: (1) GTAA-5 with signal-change trading only, (2) dual momentum, (3) capped volatility overlay on one equity ETF, (4) BAA-G4 as a deliberately skeptical inclusion, since its parameters were tuned.
- Cross-asset selection in dual momentum and BAA has a cross-sectional component. Time-series filters such as GTAA-5 are the most distinct from the already rejected stock momentum tests.
- A 20% sleeve of about USD 492 may be smaller than one share of some ETFs, so fractional shares or lower-priced equivalents are needed.

### Gaps
- Not verified: whether this account can buy US-domiciled ETFs. EU retail clients are commonly restricted by PRIIPs/KID rules, which would force UCITS substitutes with shorter or different histories. This needs checking before any live use.
- Not verified: IBKR fractional-share eligibility, and FX conversion cost for a EUR-base account buying USD instruments. FX cost could exceed the commission cost calculated above.

## Exact rule definitions for a pre-registered backtest with daily OHLCV

### Takeaway
All rules below can be computed from month-end adjusted closes, with execution assumed at the next session's open. The published versions trade at the signal-day close, which is not implementable, and HOP's evidence shows delay reduces performance, so the delayed fill should be the primary test.

### Cited Findings
- Faber: buy when monthly price exceeds the 10-month SMA, otherwise move to cash; updated only on the last day of the month; entry and exit at that day's close; total-return series; T-bills as cash; Yahoo users need adjusted prices. — [Faber](https://mebfaber.com/wp-content/uploads/2016/05/SSRN-id962461.pdf)
- MOP: return is sign(r over t-12 to t) x (40% / sigma_t) x r over t to t+1. Sigma squared is 261 x the sum of (1-d) d^i (r_(t-1-i) - rbar_t)^2, with d/(1-d) = 60 days of center of mass. — [MOP](https://pages.stern.nyu.edu/~lpederse/papers/TimeSeriesMomentum.pdf)
- HOP: signal is the sign of past excess return over 1, 3 and 12 months; positions sized for equal volatility. — [HOP](https://fairmodel.econ.yale.edu/ec439/hurst.pdf)
- Keller's SMA(12) momentum is p_t divided by the average of the 13 month-end prices p_t to p_t-12, minus 1. 13612W is 12 x RET(1) + 4 x RET(3) + 2 x RET(6) + 1 x RET(12), negative when below zero. — [Keller notes 5, 7](https://thechartist.com.au/wp-content/uploads/2024/12/5369/SSRN-id4166845.pdf)
- BAA recipe at last close of month: rank offensive and defensive assets by SMA(12) momentum; go defensive if any of SPY, VWO, VEA, BND has negative 13612W; hold Top X equal-weight; replace defensive picks with momentum below BIL's by BIL. Offensive G4 is QQQ, VWO, VEA, BND with Top 1. Defensive is TIP, DBC, BIL, IEF, TLT, LQD, BND with Top 3. — [Keller section 6, fig 6](https://thechartist.com.au/wp-content/uploads/2024/12/5369/SSRN-id4166845.pdf)
- Dual momentum is described in two orderings. One compares US and foreign 12-month returns, then requires the winner to beat cash, otherwise bonds. — [Severian guest post](https://hedgefundalpha.com/strategies/is-gary-antonaccis-global-equity-momentum-strategy-robust/) The other first requires S&P 500 12-month return to beat T-bills, then picks the stronger of S&P 500 and ex-US, otherwise bonds. The forum author says Antonacci confirmed this logic, a claim I could not check; the backtest there used SPY, VEU, SHV and BND. — [Quantopian forum archive](https://quantopian-archive.netlify.app/forum/threads/dual-momentum-investing-strategy-according-to-dr-gary-antonacci)
- Moreira and Muir weight is c / RV^2, where RV^2 is the prior month's realized variance of daily returns (22 trading days in their notation). The capped variant is min(c / RV^2, 1). — [Moreira, Muir](https://amoreira2.github.io/alan-moreira.github.io/VolPortfolios_published.pdf)
- Cederburg et al. note that the scaling constant is not known to an investor in real time. — [Cederburg et al.](https://www.lehigh.edu/~xuy219/research/COWY.pdf)

### Inferences
These specifications are my adaptations of the cited rules. Ticker substitutions and ETF inception dates must be checked against the data.

**Common conventions**
- P_t is the dividend-adjusted close on the last NYSE trading day of month t. R(k) = P_t / P_(t-k) - 1.
- Signal at the close of t; fill at the next session's open, using Open x (AdjClose / Close). Secondary test: fill at the same close, to compare with published versions.
- Cash is BIL, earning its own return.
- Cost model: USD 0.35 per order, plus an assumed half-spread per side, doubled for the stress test.
- Trade only when the target holding changes, or when a weight drifts more than 25% from target; report the monthly full-rebalance variant separately.

**R1. GTAA-5 (Faber).** Universe SPY, EFA or VEA, IEF, DBC or GSG, VNQ. Each sleeve is 20%. SMA10 = mean(P_t ... P_(t-9)). If P_t > SMA10, hold the ETF; else hold BIL in that sleeve. Benchmark: same five ETFs equal-weight, annual rebalance. Opposite side: hold the ETF only when P_t < SMA10.

**R2. Long-only TSMOM (MOP-style).** Same universe or add GLD and TLT. Long if R(12) > R_BIL(12), else BIL; weight 1/N. Variant with volatility cap: weight = min(1/N, (0.10 / sigma_i) / N), with sigma from the EWMA above, d = 60/61, daily adjusted returns. Opposite side: long only if R(12) <= R_BIL(12).

**R3. Blended horizons (HOP-style, long-only).** Score_i = mean over k in {1, 3, 12} of 1[R(k) > R_BIL(k)]. Weight = Score_i / N, remainder in BIL. Expect more orders than R1.

**R4. Dual momentum.** Pre-register one ordering: if R_SPY(12) > R_BIL(12), hold whichever of SPY and ACWX or VEU has the higher R(12); else hold AGG or BND. Report the other ordering as a labelled robustness check. Benchmark: SPY and a 60/40 of SPY and AGG. Opposite side: hold the lower-ranked equity when the filter fails.

**R5. BAA-G4 (Keller).** Exactly as cited above. Top 1 offensive, Top 3 defensive. Run G4 and BAA-SPY only; do not retune. Note that Keller's filter pairing was in-sample optimal.

**R6. Capped volatility overlay on SPY.** Annualized RV_t = sqrt(252 x variance of daily returns in month t). Weight = min(1, 0.10 / RV_t), remainder in BIL. Using volatility rather than variance and a fixed target avoids the full-sample constant; it follows Moreira and Muir's lower-turnover realized-volatility variant. Benchmark: SPY and a constant-weight SPY/BIL mix at the same average exposure, since a capped overlay lowers market exposure.

**Evaluation notes**
- The backtest period is limited by the youngest ETF, so the common sample starts around 2008-2009 once a 12-month lookback is included. That period is almost entirely after 2010 and excludes most of the 2008 crash.
- Weekly-cluster bootstrap may understate dependence when positions are held for a month. Add a monthly block bootstrap as a check.
- Count all variants run and report them all. With six rules and their variants, a single passing variant is weak evidence.

### Gaps
- ETF inception dates were not checked in this research. The common start date above is from memory and must be confirmed from the data.
- Keller's pre-ETF proxy method was not read, so pre-ETF extensions of R5 cannot be reproduced from these notes.
- The half-spread assumption for each ETF needs measuring from IBKR quotes.
