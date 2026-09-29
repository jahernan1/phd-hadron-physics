# packages/montecarlo

## What this is

Pins and patches for the thesis Monte Carlo chain (`gen_amp_V2` ->
`hdgeant4` -> `mcsmear` -> `hd_root`, driven by MCwrapper). JLab code is
never vendored into this repository: `external.lock` records exactly which
upstream commit each dependency is pinned to and, for the two dependencies
the author edited, the patches needed to reproduce those edits and the
sha256 of every file they touch.

## Pins

| name | upstream | ref | sha (12 chars) | fetched? |
| --- | --- | --- | --- | --- |
| halld_sim | https://github.com/JeffersonLab/halld_sim | 4.54.0 | `bcff7a5c4e84` | yes |
| gluex_MCwrapper | https://github.com/JeffersonLab/gluex_MCwrapper | c4e918a2 | `c4e918a2fe0e` | yes |
| AmpTools | https://github.com/mashephe/AmpTools | v0.15.2 | `fed181949540` | no (GlueX version set) |
| HDGeant4 | https://github.com/JeffersonLab/HDGeant4 | 2.42.0 | `e09d41fad180` | no (GlueX version set) |

`fetch: false` means the package is provided by the GlueX version set
(`env/version_sets/*.xml.in`); the pin here is for the record only.

## Patches

**halld_sim** (5 patches, applied on top of `4.54.0` / `bcff7a5c4e84...`):

1. `Hist2D: add CosTheta histTypes` — adds `CosThetaVsEgamma`,
   `CosThetaVst` and `CosThetaVsMass` (cos theta of the second decay
   particle, index 3 of the reaction, in the rest frame of the particle-list
   resonance; axes are built from the lab-frame resonance and beam
   directions with no centre-of-mass boost, the pseudo-helicity frame of the
   analysis note); `t` for `MassVst` becomes `|(beam-recoil)^2|`.
2. `AMPTOOLS_AMPS: add Hist3D amplitude` — samples a 3D histogram (mass,
   cos theta, t) the way `Hist2D` samples a 2D one.
3. `gen_amp_V2: register Hist3D, lvRange recoil histograms, costheta
   diagnostics` — registers `Hist3D`; `m_recoil`/`mW_recoil` use the
   `-lvRange` limits; adds `M_CosTheta`/`MW_CosTheta` diagnostics;
   diagnostic `t` is `-(beam-resonance)^2` and `intenWVsM` uses the recoil
   mass. Generated events are unchanged.
4. `gen_amp: force K+ for the suffixed second particle` — the reaction
   line names the second kaon `K+%2`, which `ParticleEnum` does not parse;
   sets `Particles[2]=KPlus` and prints each particle name.
5. `mcsmear: revert FCALSmearer channel loop for MC recon` — loops over
   the CCDB `block_mc_efficiency` table size instead of
   `fcalGeom->numFcalChannels()`, rolling back upstream
   `c56a7e520a18f95f8daac0aa533e440be91eda48` as needed for the recon
   launches used here.

`Hist2D_MultiPart.cc`, present in the source area, is an unused orphan
that nothing builds and is deliberately not included.

**gluex_MCwrapper** (2 patches, applied on top of `c4e918a2fe0e48...`):

1. `Add UPPER/LOWER_VERTEX_INDICES config keys for gen_amp_V2 -uv/-lv` —
   `gluex_MC.py` reads `UPPER_VERTEX_INDICES` (default 1) and
   `LOWER_VERTEX_INDICES` (default 23) and passes them after
   `GEN_MAX_ENERGY`; `MakeMC.sh`/`.csh` shift them in and pass `-uv`/`-lv`
   to `gen_amp_V2`.
2. `geant4: force Lambda -> p pi- in run.mac` — sets the Lambda decay
   branching ratios to 1 (p pi-) and 0 (n pi0) before `/run/beamOn`. The
   bash copy uses `>>` (the csh-only `>>!` wrote to a file named `!` under
   bash).

## Fetch and verify

```bash
gxana externals fetch halld_sim --dest "$GXANA_EXTERNALS/halld_sim-recon-2018_08-ver02_31"
gxana externals status
```

(Task 2.) Then build:

```bash
packages/montecarlo/scripts/build_halld_sim.sh recon-2018_08-ver02_31
```

(Task 3.)

## Provenance

Patches and configs were extracted from the author's thesis MC production
(halld_sim 4.54.0 + edits; MCwrapper c4e918a2 + edits).
