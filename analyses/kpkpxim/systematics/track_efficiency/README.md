# Track-efficiency systematic (dissertation chapter 7)

Data-versus-MC comparison of the track kinematics (K⁺₁, K⁺₂, π⁻₁, π⁻₂,
proton) and the resulting total track-efficiency difference used in the
external-systematics table.

## Run (two steps)

```sh
mkdir -p $GXANA_OUTPUT/kpkpxim/systematics/track_efficiency
cd $GXANA_OUTPUT/kpkpxim/systematics/track_efficiency
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/systematics/track_efficiency/get_hists.C
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/systematics/track_efficiency/get_track_efficiency.C
```

1. `get_hists.C` (`get_hists()`) reads, per period stem
   (`kpkpxim__M23_2017-01_ana56`, `kpkpxim__B4_M23_2018-01_ana03`,
   `kpkpxim__B4_M23_2018-08_ana02`):
   the Q-factor output
   `$GXANA_OUTPUT/kpkpxim/qfactors/<stem>_nominal_kphighrap_1111111/postQVal_flatTree_<stem>_nominal_kphighrap_1111111.root`
   (data), `$GXANA_DATA/flatTrees/flatTree_<stem>_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root`
   (MC) and `$GXANA_DATA/flatTrees/flatTree_thrown_<stem>_gen_amp_V2_ac_YstarRest.root`
   (thrown; copied from `rawTrees/` as in the channel README). It writes
   `particle_kinematics.root` in the current directory.
2. `get_track_efficiency.C` (`get_track_efficiency()`) reads
   `particle_kinematics.root`, prints per-particle `(Data, MC)` efficiencies
   and `Total Track Efficiency: (data, MC)`, and writes
   `<hist>_data_mc.pdf` (particle kinematics and angle distributions, data
   versus MC) to the current directory.

The printed total track efficiency and its data/MC difference feed the
track-efficiency row of the chapter 7 external-systematics table; the PDFs are
the kp1/kp2/pim1/pim2/proton kinematic figures.

## WeightMC.C

`WeightMC(Bool_t save=true)` is a copy of
`../mc_weight_variations/WeightMC.C`; it is not part of the two-step run
above. Its defaults point at the older `Ystar2400_1600_genr8` MC and a
`ver56` stem, so it cannot run on the preserved data (see
[`../mc_weight_variations/README.md`](../mc_weight_variations/README.md)).
