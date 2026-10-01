# systematics

The systematic studies of the cross section, run as `gxana run systematics --channel <ch>`.
Configured by `analyses/<channel>/config/systematics.yaml` (a `systematics:` block, with
periods, binning, trees and flux taken from `xsection.yaml`); another analysis reuses the
package by writing that one file. Numbers are computed in Python (`gxana_systematics`);
drawing is C++ ROOT (`gxana_syst_plot`, `gxana_syst_track`); no C++ app writes a number into
a stats file.

```sh
gxana run systematics --channel kpkpxim [--steps fit,qvalue,weight,spread,track,summary] [--study NAME ...] [--dry-run]
```

`--study` restricts the run to the named studies (and the variants they need);
`--dry-run` prints the commands.

## Steps

| Step | Runs | Output (under `systematics.output_dir`) |
|---|---|---|
| `fit` | `gxana_xsec_tables` per variant group | `variants/data/<label>/` |
| `qvalue` | `gxana_xsection.qvalue_rescale` | `variants/data/<qvalue label>/` |
| `weight` | `gxana_xsection.weighted_average` | `variants/weighted_data/<label>/` |
| `spread` | `gxana_systematics.{spread,sfactor}` + `gxana_syst_plot` | `<study>/` |
| `track` | `gxana_syst_track` + `gxana_systematics.track` | `<study>/` |
| `runperiod` (opt-in) | `<channel>/<runperiod.macro>` | `runperiod/` |
| `compare` (opt-in) | `gxana_systematics.runcompare` + `gxana_syst_plot` | `<study>/` |
| `summary` | `gxana_systematics.summary` | `summary/` |

The default steps are `fit, qvalue, weight, spread, track, summary`; `runperiod` and `compare`
are opt-in checks that are skipped with a note while the labels or tables they read do not
exist. The nominal tables are `xsection/weighted_data/<nominal>/` of the xsection stage
(`gxana run xsection --steps bin,tables,weight`), which must exist first.

## Study kinds

| Kind | Meaning | Outputs |
|---|---|---|
| `spread` | mean and sample standard deviation over the weighted tables of the listed labels, per point | `<stats>` (header `XVal XErr YMean StdDev`, values `%.6g`), `plots/<name>.pdf` per plot, optional example fits |
| `sfactor` | PDG scale factor S of the run-period combination and the run systematic built from it (D9) | `<stats>` with columns `XVal XErr YMean StatErr Chi2 N S Syst` |
| `track` | track-reconstruction efficiency systematic from the data and MC track-kinematics counts | `track_counts.txt` (raw counts, `gxana_syst_track`), figures, `track_efficiency.txt` (`data`, `mc`, `data_raw`, `mc_raw`, `total` and a `report` line; `gxana_systematics.track`) |
| `constant` | a fixed relative uncertainty (`value`), e.g. the luminosity | none, read by the summary |
| `compare` | opt-in check: plots, and for the run-period comparison a stats file of the period standard deviation scaled by S | `plots/<name>.pdf`, optional `<stats>` |

The last column of each measuring study's stats file is its systematic; the xsection `tex`
step reads them through `xsection.tex.columns` (column heading -> stats file), so each
spread appears under its own heading. `xsection.tex.run_fraction` (0.051) is used only by the
`*_runsyst` study tables and has no documentary source.

## Configuration

```yaml
systematics:
  output_dir: "${GXANA_OUTPUT}/<channel>/systematics"
  nominal: johnson                 # label of xsection/weighted_data/<label>; default xsection.tex.label
  variants:                        # shared pool
    - model: JohnsonMCShape        # one gxana_xsec_tables process per group (xsection.fits schema)
      params: {...}
      labels:
        - {label: hybrid_combo, cheby: 2, weight: hybrid_combo}
    - qvalue: {label: qvalues, source: hybrid_combo}   # qvalue_rescale of a fit label's tables
  studies:
    <name>: {kind: spread|sfactor|track|constant|compare, ...}
  runperiod: {macro: systematics/<macro>.C}
  summary: {point_by_point: [<study>, ...], normalization: [<study>, ...]}
```

Unknown keys are rejected. Keys per kind (`config.STUDY_KEYS`):

| Kind | Keys |
|---|---|
| `spread` | `stats`, `spread` (at least two labels), `plots`, `examples` |
| `sfactor` | `stats` |
| `track` | `tree`, `thrown_tree`, `data_weight`, `mc_weight`, `theta_cut_deg`, `low`, `high`, `override`, `report` (`data` or `mc`), `particles`, `legend_header` |
| `constant` | `value` |
| `compare` | `plots`, `per_period`, `stats` |

Each entry of `plots` takes (`config.PLOT_KEYS`) `layout`, `name`, `labels`, `legend`
(`"text|opt"` entries), `legend_header`, `first_style` (`points` or `band`), `annotate`
(`avg:N`, `max:N`), `axis_format` (y-axis ndiv), `x_axis_format` (x-axis ndiv), `xmax`, `ymax`.
Layouts, drawn by `gxana_syst_plot` and ported verbatim from the legacy macros (now in
`archive/systematics_legacy/comparisons/`):

| Layout | Panels | Source macro |
|---|---|---|
| `grid3` | three label tables per energy bin | `PlotFitComparison.C` `PlotWeightedXSec` (same as `PlotComboComparison.C`) |
| `pair_band` | two tables and the spread band | `PlotComboComparison.C` `PlotAllXSec` |
| `all_band` | 2-8 tables and the spread band | `PlotFitComparison.C` `PlotAllXSec` |
| `grid2` | two label tables | `PlotQValueComparison.C`, `PlotBunchComparison.C`, `PlotFitBkgdComparison.C` |
| `run_grid` | the three run periods | `PlotRunComparison.C` |
| `stddev_band` | the three run periods and the scaled standard-deviation band | `PlotRunComparison.C` |

The kpkpxim `run_grid` legend names the run periods (Spring 2017, Spring 2018, Fall 2018);
the legacy macro labelled them with the combo labels copied from `PlotComboComparison.C`
("RF Sub", "Best", "Hybrid"). `summary.point_by_point` lists the studies added in quadrature
per bin (`summary/`), `summary.normalization` the studies quoted separately as one number
(track efficiency, luminosity); neither enters the point-by-point total.

## Adding a channel

Write `analyses/<channel>/config/systematics.yaml` with the block above. Nothing else: no
code in this package names a channel, period, particle or path.

## Formulas (D9)

Per (E, -t) bin, with period values x_i and statistical errors sigma_i: w_i = 1/sigma_i^2,
mean = sum w_i x_i / sum w_i, delta_stat = (sum w_i)^(-1/2), chi2 = sum w_i (mean - x_i)^2,
N = periods with nonzero weight, **S = sqrt(chi2/(N-1))** (dissertation ch. 6,
`cross_section_result.tex`). The run-period systematic is a separate rule,
**delta_syst = delta_stat (S - 1) if S > 1, else 0** (ch. 7, `internal_syst.tex`). The spread
studies use the unweighted mean and sample standard deviation of their members. The
quadrature total of `summary` is the root of the sum of the squared last columns of the
`point_by_point` studies.

## Tests

Python tests: `uv run pytest packages/systematics -q`. Goldens (`-m golden`, preserved data):
`test_systematics_text_golden.py`, `test_systematics_numbers_golden.py`,
`test_systematics_plot_golden.py`, `test_systematics_track_golden.py`,
`test_systematics_summary_golden.py` and `test_systematics_chain_golden.py` (the fit-variation
chain, `GXANA_GOLDEN_SYST_OUTPUT`, about 5-7 min).

## Differences from the published tables

Listed in [`docs/KNOWN_ISSUES.md`](../../docs/KNOWN_ISSUES.md): the Accidentals / Yield
Extraction column swap, the three-method accidentals spread (the accidentals study uses
`acc_weight`, `best_combo` and `hybrid_combo`), the track totals and the run-period check.
The Spring-2017 REST-version check (`PlotRestVComparison.C`) is not part of the suite,
pending re-evaluation.
