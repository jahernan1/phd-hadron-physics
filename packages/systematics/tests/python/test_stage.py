import copy
import sys
from pathlib import Path

import pytest

from gxana import config as gconfig
from gxana_systematics import stage as st

ENV = {"GXANA_ROOT": "/r", "GXANA_DATA": "/d", "GXANA_OUTPUT": "/o"}
OUT = "/o/kpkpxim/systematics"
XS = "/o/kpkpxim/xsection"
EDGES = ["6.40", "7.40", "7.86", "8.19", "8.45", "8.68", "9.26", "10.18", "11.40"]


def _cfg():
    return copy.deepcopy(gconfig.load_channel("kpkpxim"))


def _plan(steps, studies=None, cfg=None):
    return st.plan(cfg or _cfg(), steps, study_names=studies, environ=ENV)


def _labels(argv):
    return [argv[i + 1] for i, a in enumerate(argv) if a == "--label"]


def test_steps():
    assert st.STEPS == ("fit", "qvalue", "weight", "spread", "track", "runperiod", "compare", "summary")
    assert "compare" not in st.DEFAULT_STEPS and "runperiod" not in st.DEFAULT_STEPS


def test_fit_one_process_per_group_into_the_pool():
    cmds = _plan(["fit"])
    assert [_labels(c.argv) for c in cmds] == [
        ["hybrid_combo", "best_combo", "acc_weight"], ["johnson", "johnson_cheby1"],
        ["voigt", "voigt_cheby1"], ["mcPdf", "mcPdf_cheby1"]]
    argv = cmds[0].argv
    assert argv[argv.index("--out") + 1] == f"{OUT}/variants/data"
    assert argv[argv.index("--plots") + 1] == f"{OUT}/variants/fits"
    assert f"{XS}/binned_trees/binned_flatTree_kpkpxim__M23_2017-01_ana56_nominal_kphighrap.root" in argv[-3]


def test_study_filter_fits_only_needed_groups():
    cmds = _plan(["fit"], studies=["accidentals"])
    assert [_labels(c.argv) for c in cmds] == [["hybrid_combo", "best_combo", "acc_weight"]]
    # sfactor reads the nominal from xsection: no fit at all
    assert _plan(["fit"], studies=["run"]) == []


def test_qvalue_plans_from_existing_source_tables(tmp_path):
    cfg = _cfg()
    env = {**ENV, "GXANA_OUTPUT": str(tmp_path)}
    src = tmp_path / "kpkpxim/systematics/variants/data/hybrid_combo"
    src.mkdir(parents=True)
    for kind in ("diffout", "diffxsec"):
        (src / f"{kind}_flatTree_a_emin_6.40_emax_7.40.txt").write_text("x")
    cmds = st.plan(cfg, ["qvalue"], environ=env)
    assert len(cmds) == 1
    argv = cmds[0].argv
    assert argv[:3] == [sys.executable, "-m", "gxana_xsection.qvalue_rescale"]
    assert argv[-1] == str(tmp_path / "kpkpxim/systematics/variants/data/qvalues/diffxsec_flatTree_a_emin_6.40_emax_7.40.txt")


def test_weight_every_pool_label_per_energy_bin():
    cmds = _plan(["weight"])
    labels = {c.argv[c.argv.index("gxana_xsection.weighted_average") + 1].rsplit("/", 1)[1] for c in cmds}
    assert labels == {"hybrid_combo", "best_combo", "acc_weight", "johnson", "johnson_cheby1", "voigt",
                      "voigt_cheby1", "mcPdf", "mcPdf_cheby1", "qvalues"}
    q = [c.argv for c in cmds if c.argv[-3].endswith("/qvalues")]
    assert len(q) == len(EDGES) - 1  # qvalue tables have no totxsec
    assert all("diffxsec*_emin_" in a[-1] for a in q)


def test_label_dir():
    cfg = _cfg()
    assert st.label_dir(cfg, "voigt", ENV) == f"{OUT}/variants/weighted_data/voigt"
    assert st.label_dir(cfg, "johnson", ENV) == f"{OUT}/variants/weighted_data/johnson"
    del cfg["systematics"]["variants"][1]
    assert st.label_dir(cfg, "johnson", ENV) == f"{XS}/weighted_data/johnson"
    with pytest.raises(gconfig.ConfigError, match="'nope'"):
        st.label_dir(cfg, "nope", ENV)


@pytest.mark.parametrize("step", ["bin", "tables", "barlow"])
def test_moved_steps_point_to_run_barlow(step):
    with pytest.raises(gconfig.ConfigError, match="gxana run barlow"):
        st.plan(_cfg(), [step], environ=ENV)


def test_unknown_step():
    with pytest.raises(gconfig.ConfigError, match="unknown step 'nope'"):
        st.plan(_cfg(), ["nope"], environ=ENV)


def test_run_guard_when_nominal_missing(tmp_path, capsys):
    env = {**ENV, "GXANA_OUTPUT": str(tmp_path)}
    calls = []
    rc = st.run_systematics(_cfg(), ["fit"], runner=lambda *a, **k: calls.append(a), environ=env)
    assert rc == 1 and calls == []
    out = capsys.readouterr().err
    assert "binned_trees" in out and "gxana run xsection --channel kpkpxim --steps bin" in out


def test_dry_run_prints_and_runs_nothing(capsys):
    calls = []
    assert st.run_systematics(_cfg(), ["fit", "weight"], dry_run=True, runner=lambda *a, **k: calls.append(a),
                              environ=ENV) == 0
    assert calls == [] and "gxana_xsec_tables" in capsys.readouterr().out


def test_stop_on_first_failure(tmp_path, monkeypatch):
    class Fail:
        returncode = 3
    monkeypatch.setattr(st, "preflight", lambda *a, **k: [])
    calls = []
    rc = st.run_systematics(_cfg(), ["fit"], runner=lambda argv, **k: calls.append(argv) or Fail(),
                            environ={**ENV, "GXANA_OUTPUT": str(tmp_path)})
    assert rc == 3 and len(calls) == 1
