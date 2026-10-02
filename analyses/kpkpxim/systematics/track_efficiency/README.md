# Track-efficiency systematic (dissertation chapter 7)

Data-versus-MC comparison of the track kinematics (K⁺₁, K⁺₂, π⁻₁, π⁻₂,
proton) and the resulting total track-efficiency uncertainty quoted in the
external-systematics section (`tab:track_eff`).

## Run

```sh
gxana run systematics --channel kpkpxim --steps track --study track
```

The `track` study of `config/systematics.yaml` sets the trees, weights, the
θ cut (20°), the per-track uncertainties below/above it (3 % / 5 %), the
proton override and the particles (p4 branches, binning, axis titles). The
inputs per period are the `xsection.inputs` of `config/xsection.yaml`: the
Q-factor output (data, weight `qvalue_decayxim_M*hybrid_combo`; from
`gxana run qfactors`), and the `mc_sample` reconstructed (weight
`hybrid_combo`) and thrown flat trees under `$GXANA_DATA/flatTrees` (the
thrown trees are copied from `rawTrees/` as in the channel README).

Two commands run, writing to `$GXANA_OUTPUT/kpkpxim/systematics/track/`:

1. `gxana_syst_track` fills `particle_kinematics.root` (one directory per
   period with `<name>_kin_{qval,mc,thrown}`, the acceptance and the
   acceptance-corrected histograms; the merged `<name>_kin_phase1`,
   `_phase1_mc`, `_phase1_acccorr`), draws the data/MC angle figures
   `<name>_kin_angle_phase1_mc_data_mc.pdf` (the ε annotation is the MC
   value) and writes the data and MC counts below/above the cut to
   `track_counts.txt`.
2. `python -m gxana_systematics.track` writes `track_efficiency.txt`: per
   track the data and MC values (`(0.03 N_low + 0.05 N_high) / N`, or the
   override), both raw values, their sums and the reported total.

## Which numbers the text uses

The dissertation uses the signal MC (`report: mc`; data gives very similar
values) and assigns the proton the conservative GlueX 5 % (`override:
{proton: 0.05}`); the computed proton value (3.29 %) is quoted in
parentheses. The per-track MC values (3 %, 4.94 %, 3.83 %, 3.59 %, 3.29 %)
reproduce the table, but its totals 20.29 % (18.58 % with the computed
proton) are not the sum of the per-track values, which is 20.36 % (18.65 %);
`track_efficiency.txt` reports the sums.

## Legacy macros

`get_hists.C` and `get_track_efficiency.C` are archived in
`archive/systematics_legacy/track_efficiency/` (the golden test
`tests/golden/test_systematics_track_golden.py` runs them to compare the
figures).

## WeightMC.C

The copy of `WeightMC.C` that sat here was byte-identical to
[`../mc_weight_variations/WeightMC.C`](../mc_weight_variations/WeightMC.C) and was removed on
2026-10-02; use that one (see [`../mc_weight_variations/README.md`](../mc_weight_variations/README.md)).
