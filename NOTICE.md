# NOTICE

The author's code in this repository is MIT-licensed (see `LICENSE`). The
projects below are upstream work by others: they are credited here, keep
their own authorship and licenses, and are **not** covered by `LICENSE`.

## Pinned, patched, not redistributed (Monte-Carlo)

`packages/montecarlo/external.lock` pins these; `gxana externals fetch` clones
them. Patches in `packages/montecarlo/patches/` are modifications by
J. A. Hernandez to the listed upstream revision. Details and exact revisions:
[`packages/montecarlo/NOTICE.md`](packages/montecarlo/NOTICE.md).

- halld_sim — https://github.com/JeffersonLab/halld_sim (Jefferson Lab, GlueX)
- gluex_MCwrapper — https://github.com/JeffersonLab/gluex_MCwrapper (Jefferson Lab, GlueX)
- AmpTools — https://github.com/mashephe/AmpTools (M. Shepherd et al.)
- HDGeant4 — https://github.com/JeffersonLab/HDGeant4 (Jefferson Lab, GlueX)

## QFactors (git submodule `packages/qfactors`)

Q-factor engine by L. Ng: https://github.com/lan13005/QFactors (upstream
`d130c05`). This repository uses the fork https://github.com/jahernan1/QFactors
(branch `kpkpxim-thesis`), whose added commits are listed in its
`CHANGES_THESIS.md`. The upstream repository publishes no license file; all
original authorship is retained and the fork is not relicensed by this
repository's `LICENSE`.

## GlueX software used, not included

- gluex_root_analysis (Jefferson Lab): the DSelectors in `analyses/*/selectors/`,
  `analyses/kpkpxim/backgrounds/selectors/`, and `archive/selectors/` were
  generated from its DSelector template and keep the template's header
  comments; `DPROOFLiteManager` runs them.
- GlueX version sets (`halld_versions`) and the GlueX container images, used
  unmodified (`docs/environment.md`); `env/version_sets/*.xml.in` are templates
  of the upstream recon version sets.

## Upstream files kept in `archive/`

`archive/` keeps legacy copies verbatim for provenance. These files are, or
derive from, upstream code and keep their upstream authorship and license:

- `archive/mc_legacy/jlab/gen_amp/MakeMC.csh`, `archive/mc_legacy/jlab/gen_amp/MakeMC_old.csh`,
  `archive/mc_legacy/local/MakeMC.csh` — gluex_MCwrapper (https://github.com/JeffersonLab/gluex_MCwrapper)
- `archive/mc_legacy/jlab/gen_amp_mod.cc` — halld_sim `gen_amp` (https://github.com/JeffersonLab/halld_sim)

Configuration files written for these tools (AmpTools `.cfg`, MCwrapper
`.config`) are the author's own.

## Published data

- `analyses/kpkpxim/xsection/external_data/Clas_data.csv`: CLAS measurement of
  γp → K⁺K⁺Ξ⁻, used for comparison only. Source: `<FILL: CLAS publication reference>`.

No GlueX data are included in this repository.
