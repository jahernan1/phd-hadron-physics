# Background channels

Reactions that feed down into, or share final-state particles with,
γp → K⁺K⁺Ξ⁻, studied to understand the background under the Ξ⁻ peak.

| Channel | Selectors (`selectors/`) | `dOutputFileName` |
|---|---|---|
| γp → π⁰K⁺K⁺Ξ⁻ | `DSelector_pi0kpkpxim.C/.h` | `pi0kpkpxim.root` (flat tree `flatTree_pi0kpkpxim.root`) |
| γp → π⁺π⁻K⁺Λ | `DSelector_pippimkplamb.C/.h`, `DSelector_thrown_pippimkplamb.C/.h` | `pippimkplamb.root`, `thrown_pippimkplamb.root` |

Fit macros: `KstarFit.C` (K* contribution), `YstarBWFitsData.C`
(Breit–Wigner fits to Y* → K⁺Ξ⁻ structures in data).

Like the main selectors, these need the GlueX environment to load.

## How to run

Output and input directories follow the channel README (`$GXANA_OUTPUT`,
`$GXANA_DATA`).

### Selectors

`backgrounds/selectors/` holds `DSelector_pi0kpkpxim`,
`DSelector_pippimkplamb` and `DSelector_thrown_pippimkplamb`. They are not
listed in `config/samples.yaml`, and `gxana run select` looks for the output
file named by `output_basename` in `config/channel.yaml` (`kpkpxim.root`), not
the `dOutputFileName` of these selectors, so a full `gxana run select` does
not collect their output. Use the stage to print the plan and the PROOF-Lite
script, then run it by hand with your own tree directory in the GlueX
environment:

```sh
gxana run select --channel kpkpxim --period 2018-08 --sample data --selector $GXANA_ROOT/analyses/kpkpxim/backgrounds/selectors/DSelector_pi0kpkpxim.C --dry-run
```

The input trees must be the analysis trees that contain the reaction of the
selector (π⁰K⁺K⁺Ξ⁻ or π⁺π⁻K⁺Λ); the π⁰ MC variant of the hd_root config is
`simulation/hd_root/hd_root_xim_pi0_2018.conf`. Which skim produced the
preserved background trees was not checked.

### Fit macros

- `YstarBWFitsData.C`: entry `YstarBWFitsData()` runs
  `FitYstarMass(stem)` for the stem hard-coded in the macro
  (`kpkpxim__B4_M23_2018-08_ana02_nominal_allKaonSep_1111111`); the stem must
  be an existing Q-factor directory
  `$GXANA_OUTPUT/kpkpxim/qfactors/<stem>/postQVal_flatTree_<stem>.root`. Edit
  the stem to a `..._nominal_kphighrap_1111111` directory produced by
  `gxana run qfactors`, then:

  ```sh
  mkdir -p $GXANA_OUTPUT/kpkpxim/backgrounds
  cd $GXANA_OUTPUT/kpkpxim/backgrounds
  root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/backgrounds/YstarBWFitsData.C
  ```

  It prints the Breit-Wigner fit yields.
- `KstarFit.C`: cannot run as preserved. It reads the legacy
  `flatTree_kpkpxim__M23_2017-01_ver56_allCuts.root` (tree `kpkpxim_flatTree`)
  and a legacy Q-factor directory, neither of which is in the preserved data.

