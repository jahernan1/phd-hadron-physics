# Preserved analysis data

GlueX practice for documenting dissertation work: the code is published on
GitHub and the data it analysed is preserved on disk under
`/work/halld/gluex_analysis_data/` at JLab. This repository follows the same
split — code in git, data under `$GXANA_ANALYSIS_DATA`, never committed.

| Site | `GXANA_ANALYSIS_DATA` |
|---|---|
| default (laptop) | `$GXANA_ROOT/gluex_analysis_data` — gitignored; may be a symlink |
| JLab | the thesis directory under `/work/halld/gluex_analysis_data/`, set in `env/site.sh` |

Each channel keeps its files in `$GXANA_ANALYSIS_DATA/<channel>/` and documents
them in `analyses/<channel>/analysis_data.yaml`: a description per directory
and the sha256 + size of every file.

## kpkpxim

Tree stems `<P>`: `kpkpxim__M23_2017-01_ana56`, `kpkpxim__B4_M23_2018-01_ana03`,
`kpkpxim__B4_M23_2018-08_ana02`.

    kpkpxim/
      flat_trees/     postQVal_flatTree_<P>_nominal_kphighrap_1111111.root
                      flatTree_<P>_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root
                      flatTree_thrown_<P>_gen_amp_V2_ac_YstarRest.root
      binned_trees/   binned_flatTree_<P>_nominal_kphighrap.root
                      binned_flatTree_<P>_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root
                      binned_thrown_flatTree_<P>_gen_amp_V2_ac_YstarRest.root
      flux/           flux_30274_31057_r4.root flux_40856_42559.root flux_50685_51768.root
      reference/xsection/
        <label>/ weighted/<label>/ components/{sp17,sp18,fa18}/<label>/
          hybrid_combo  thesis fit (legacy MakeXSecFiles.C), weight hybrid_combo: dissertation result
          best_combo    thesis fit, weight best_combo  (combo-selection study)
          acc_weight    thesis fit, weight acc_weight  (combo-selection study)
          johnson       fit-model variation (legacy MakeXSecFitVariations.C), weight hybrid_combo
        tables/         thesis LaTeX tables from weighted/hybrid_combo
        qvalues/

## Commands

    gxana data path   --channel kpkpxim   # where the files are expected
    gxana data status --channel kpkpxim   # ok | open (not locked) | miss | diff | new
    gxana data lock   --channel kpkpxim   # record sha256 + bytes, then commit the manifest

## Golden tests

`tests/golden/` rerun the cross-section stages on these files and compare with
the legacy reference outputs (spec D20); each test skips when its files are
absent. Build first (`uv run cmake --build build`), then `uv run pytest -m golden -v`.

- binning (`gxana_xsec_bin`): tree names, entry counts and branch sums must match exactly;
- fits (`gxana_xsec_tables`): relative tolerance `GXANA_GOLDEN_RTOL` (default `1e-5`).
  The authoritative run is in the analysis container (ROOT 6.24.04, as for the
  thesis); on newer ROOT record the reported maximum deviation instead;
- Python (`gxana_xsection`): weighted average to the printed 6 decimals, components
  and Q-value rescale to `1e-12`, LaTeX tables byte-identical.

## Depositing at JLab

    rsync -av gluex_analysis_data/kpkpxim/ /work/halld/gluex_analysis_data/<thesis-dir>/kpkpxim/
    # env/site.sh:  export GXANA_ANALYSIS_DATA=/work/halld/gluex_analysis_data/<thesis-dir>
    gxana data status --channel kpkpxim   # expect every file ok
