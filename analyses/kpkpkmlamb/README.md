# kpkpkmlamb — excited Ξ* in γp → K⁺K⁺K⁻Λ

Side channel of the thesis. The main measurement (`analyses/kpkpxim`) extracts
the ground-state Ξ⁻(1320) in γp → K⁺K⁺Ξ⁻. Here the same GlueX-I data are
searched for excited cascades decaying to K⁻Λ, Ξ(1690)⁻ and Ξ(1820)⁻, in
γp → K⁺K⁺K⁻Λ (Λ → pπ⁻). The signals are small; the point of the channel is
to show that the framework built for the main analysis (selection, flat trees,
fits) extracts a rare signal the same consistent way. Results are in the
excited-Ξ chapter of the dissertation (see [`CITATION.cff`](../../CITATION.cff)).

## Decisions reused from kpkpxim

| Step | kpkpkmlamb | as in kpkpxim |
|---|---|---|
| Selector | `DSelector_kpkpkmlamb` from the gluex_root_analysis template, run with `gxana run select` | same framework |
| Beam energy | 6.4 < E_γ < 11.4 GeV | same window |
| Combo choice | best χ²/ndf combo (`best_combo==1`) | same |
| Exclusivity | \|MM²\| < 0.02 GeV², χ²/ndf < 3 | same cut values |
| Target | 50.4 < z_vertex < 79.1 cm | same |
| Λ | path-length significance > 0, 1.107 < M(pπ⁻) < 1.125 GeV | Λ/Ξ detached-vertex cuts |
| Kaon ordering | M(K⁺K⁻) > 1.1 GeV for both K⁺ (removes φ), fast-K⁺ rapidity > 0 in the fit | fast-K⁺ rapidity cut |

Periods: 2017-01 (`ana55`), 2018-01 (`ana22`), 2018-08 (`ana19`), fit prefix
`B4_M18_` (`config/periods.yaml`). Data only: no MC samples, no Q-factors, no
cross section.

## Pipeline

```bash
source env/setup.sh --gluex          # selector step needs the GlueX environment
for p in 2017-01 2018-01 2018-08; do
  uv run gxana run select --channel kpkpkmlamb --period $p --sample data
done                                  # → $GXANA_DATA/Trees/flatTree/rawTrees/flatTree_kpkpkmlamb__B4_M18_<period>_<launch>.root

root -l -b -q rootlogon.C analyses/kpkpkmlamb/flat_trees/flatTreePrep.C
                                      # nominal cuts → $GXANA_DATA/kpkpkmlamb/*_nominal_allCuts.root
hadd -f $GXANA_DATA/kpkpkmlamb/flatTree_kpkpkmlamb_GlueX-I.root $GXANA_DATA/kpkpkmlamb/flatTree_kpkpkmlamb__B4_M18_*_nominal_allCuts.root

uv run gxana run measurements --channel kpkpkmlamb   # FitXimStar.C(4, false|true) in $GXANA_OUTPUT/kpkpkmlamb/measurements:
                                                     # Xi1820massFit.pdf, Xi1820massFit_TCut3.pdf (t_dist > 1)
# by hand:
mkdir -p $GXANA_OUTPUT/kpkpkmlamb/measurements && cd $GXANA_OUTPUT/kpkpkmlamb/measurements
root -l -b -q $GXANA_ROOT/rootlogon.C '$GXANA_ROOT/analyses/kpkpkmlamb/measurements/FitXimStar.C(4, false)'
root -l -b -q $GXANA_ROOT/rootlogon.C '$GXANA_ROOT/analyses/kpkpkmlamb/measurements/FitXimStar.C(4, true)'
```

The fit: extended RooFit model, Breit-Wigner Ξ(1820)⁻ and Ξ(1690)⁻ (a Ξ(1620)⁻
term is present with its yield fixed to 0) on a Chebychev background, 150 bins
in 1.6–2.6 GeV, with a pull panel.

## Directory map

| Path | Contents |
|---|---|
| `config/` | channel, periods, samples (`Trees/kpkpkmlamb/tree_<stem>/trees/`), measurements (`measurements.yaml`: the `FitXimStar.C` runs of `gxana run measurements`) |
| `selectors/` | `DSelector_kpkpkmlamb.{C,h}` + README |
| `flat_trees/flatTreePrep.C` | nominal cuts; `tests/make_raw_tree.C` toy input |
| `measurements/FitXimStar.C` | Ξ* mass fit; `tests/make_toy_tree.C` toy input |

## Legacy provenance

`FitXimStar.C` merges the legacy `FitXimStar.C` and `FitXimStarCuts.C`
(`tCut` argument). Edits made on migration are marked `// gxana:` in the code.
