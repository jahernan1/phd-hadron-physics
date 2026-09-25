from pathlib import Path

import pytest

from gxana import paths


def test_env_path_joins_parts():
    env = {"GXANA_DATA": "/data"}
    assert paths.env_path("GXANA_DATA", "Trees", "x.root", environ=env) == Path("/data/Trees/x.root")


def test_env_path_without_parts_returns_root():
    assert paths.env_path("GXANA_SCRATCH", environ={"GXANA_SCRATCH": "/s"}) == Path("/s")


def test_env_path_missing_raises():
    with pytest.raises(paths.MissingEnvError, match="GXANA_OUTPUT is not set"):
        paths.env_path("GXANA_OUTPUT", environ={})


def test_env_path_empty_value_counts_as_missing():
    with pytest.raises(paths.MissingEnvError):
        paths.env_path("GXANA_DATA", environ={"GXANA_DATA": ""})


def test_env_path_rejects_unknown_var():
    with pytest.raises(ValueError, match="unknown variable"):
        paths.env_path("HOME", environ={"HOME": "/x"})


def test_repo_root_defaults_to_checkout(monkeypatch):
    monkeypatch.delenv("GXANA_ROOT", raising=False)
    assert (paths.repo_root() / "docs" / "REFACTOR_SPEC.md").is_file()


def test_repo_root_honours_env(monkeypatch, tmp_path):
    monkeypatch.setenv("GXANA_ROOT", str(tmp_path))
    assert paths.repo_root() == tmp_path


@pytest.mark.parametrize(
    "legacy,expected",
    [
        ("/d/grid17/hjesse/KpKpKmL012017012018082018Real_31July.root",
         "${GXANA_DATA}/KpKpKmL012017012018082018Real_31July.root"),
        ("/d/grid17/hjesse/AnalysisNote/flatTrees/flatTree_a.root", "${GXANA_DATA}/flatTrees/flatTree_a.root"),
        ("/d/grid17/hjesse/AnalysisNote/QFactors/logs/run1/", "${GXANA_OUTPUT}/kpkpxim/qfactors/run1/"),
        ("/d/grid17/hjesse/AnalysisNote/fluxFiles/flux_2017.root", "${GXANA_DATA}/flux/flux_2017.root"),
        ("/d/grid17/hjesse/AnalysisNote/xsection/data/", "${GXANA_OUTPUT}/kpkpxim/xsection/data/"),
        ("/d/grid17/hjesse/Trees/flatTree/rawTrees/", "${GXANA_DATA}/Trees/flatTree/rawTrees/"),
        ("/d/grid17/hjesse/analysis/kpkpxim/kpkpxim__B4_M23_2018-08_ana02.root",
         "${GXANA_OUTPUT}/kpkpxim/selector_hists/kpkpxim__B4_M23_2018-08_ana02.root"),
        ("/d/grid17/hjesse/analysis/other_channel/foo.root",
         "${GXANA_OUTPUT}/kpkpxim/selector_hists/other_channel/foo.root"),
        ("/d/grid17/hjesse/macros/yields/", "${GXANA_OUTPUT}/legacy_macros/yields/"),
        ("/d/grid17/hjesse/temp", "${GXANA_SCRATCH}"),
        ("/d/grid17/hjesse/Clas_data.csv", "${GXANA_ROOT}/analyses/kpkpxim/xsection/external_data/Clas_data.csv"),
        ("relative/path.root", "relative/path.root"),
    ],
)
def test_legacy_to_env(legacy, expected):
    assert paths.legacy_to_env(legacy) == expected


def test_analysis_data_root_from_env():
    assert paths.analysis_data_root({"GXANA_ANALYSIS_DATA": "/w/gad"}) == Path("/w/gad")


def test_analysis_data_root_defaults_to_repo(monkeypatch, tmp_path):
    monkeypatch.setenv("GXANA_ROOT", str(tmp_path))
    assert paths.analysis_data_root({}) == tmp_path / "gluex_analysis_data"
    assert paths.analysis_data_root({"GXANA_ANALYSIS_DATA": ""}) == tmp_path / "gluex_analysis_data"


def test_env_path_accepts_analysis_data():
    env = {"GXANA_ANALYSIS_DATA": "/g"}
    assert paths.env_path("GXANA_ANALYSIS_DATA", "kpkpxim", environ=env) == Path("/g/kpkpxim")


def test_legacy_flux_prefix():
    from gxana.paths import legacy_to_env
    assert (legacy_to_env("/d/grid17/hjesse/analysis/kpkpxim/flux/flux_40856_42559.root")
            == "${GXANA_DATA}/flux/flux_40856_42559.root")


def test_kpkpkmlamb_prefix_maps_to_data():
    from gxana.paths import legacy_to_env
    assert legacy_to_env("/d/grid17/hjesse/kpkpkmlamb/x_nominal_allCuts.root") == "${GXANA_DATA}/kpkpkmlamb/x_nominal_allCuts.root"
