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
    assert (paths.repo_root() / "docs" / "history" / "REFACTOR_SPEC.md").is_file()


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


def test_gxana_root_prefers_the_given_mapping(monkeypatch):
    monkeypatch.setenv("GXANA_ROOT", "/from-os")
    assert paths.gxana_root({"GXANA_ROOT": "/given"}) == Path("/given")


def test_gxana_root_falls_back_to_repo_root(monkeypatch):
    monkeypatch.delenv("GXANA_ROOT", raising=False)
    checkout = Path(paths.__file__).resolve().parents[4]
    assert paths.gxana_root() == checkout
    assert paths.gxana_root({}) == checkout
    assert paths.gxana_root({"GXANA_ROOT": ""}) == checkout


def test_gxana_root_reads_os_environ_when_the_mapping_lacks_it(monkeypatch):
    # Kept behaviour (docs/history/PORT_NOTES.md): an explicit mapping cannot hide a set GXANA_ROOT.
    monkeypatch.setenv("GXANA_ROOT", "/from-os")
    assert paths.gxana_root({"GXANA_DATA": "/d"}) == Path("/from-os")


def _fake_checkout(base):
    (base / "env").mkdir(parents=True)
    (base / "env" / "setup.sh").write_text("")
    module = base / "packages" / "common" / "python" / "gxana" / "paths.py"
    module.parent.mkdir(parents=True)
    module.write_text("")
    return base, module


def test_foreign_checkout_clean(tmp_path):
    root, module = _fake_checkout(tmp_path / "a")
    assert paths.foreign_checkout(root, environ={}, module_file=module) == []
    assert paths.foreign_checkout(root, environ={"GXANA_ROOT": str(root)}, module_file=module) == []


def test_foreign_checkout_env_and_module(tmp_path):
    a, _ = _fake_checkout(tmp_path / "a")
    b, module_b = _fake_checkout(tmp_path / "b")
    problems = paths.foreign_checkout(a, environ={"GXANA_ROOT": str(b)}, module_file=module_b)
    assert len(problems) == 2
    assert any("GXANA_ROOT" in p for p in problems) and any(str(b) in p for p in problems)


def test_foreign_checkout_ignores_non_checkout_install(tmp_path):
    a, _ = _fake_checkout(tmp_path / "a")
    site = tmp_path / "site-packages" / "x" / "y" / "gxana" / "paths.py"
    site.parent.mkdir(parents=True)
    site.write_text("")
    assert paths.foreign_checkout(a, environ={}, module_file=site) == []
