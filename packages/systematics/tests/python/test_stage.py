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


def _mod(argv):
    return argv[2] if argv[:2] == [sys.executable, "-m"] else None


def test_spread_step_order_and_members():
    cmds = [c.argv for c in _plan(["spread"])]
    mods = [_mod(a) for a in cmds]
    # run (sfactor) first: file order of studies
    assert mods[0] == "gxana_systematics.sfactor"
    s = cmds[0]
    assert s[s.index("--periods-dir") + 1] == f"{XS}/data/johnson"
    assert s[s.index("--n-periods") + 1] == "3"
    assert [s[i + 1] for i, a in enumerate(s) if a == "--energy"] == [f"{a}:{b}" for a, b in zip(EDGES, EDGES[1:])]
    assert s[s.index("--out") + 1] == f"{OUT}/run/sfactor_stats.txt"
    # the pool refits johnson: check it equals the nominal before any spread uses it
    same = [a for a in cmds if _mod(a) == "gxana_systematics.tables"]
    assert same[0][-2:] == [f"{OUT}/variants/weighted_data/johnson", f"{XS}/weighted_data/johnson"]
    acc = next(a for a in cmds if _mod(a) == "gxana_systematics.spread" and "combo_variations" in a[a.index("--out") + 1])
    assert acc[acc.index("--out") + 1] == f"{OUT}/accidentals/combo_variations_stats.txt"
    assert [acc[i + 1] for i, a in enumerate(acc) if a == "--member"] == [
        f"{l}={OUT}/variants/weighted_data/{l}" for l in ("acc_weight", "best_combo", "hybrid_combo")]


def test_spread_copies_example_fits():
    cps = [c.argv for c in _plan(["spread"], studies=["fit"]) if c.argv[0] == "cp"]
    assert len(cps) == 6
    assert cps[0] == ["cp", f"{OUT}/variants/fits/johnson/data_flatTree_kpkpxim__B4_M23_2018-08_ana02_emin_8.45_emax_8.68.pdf",
                      f"{OUT}/fit/plots/fit_examples/johnsonFit.pdf"]


def test_spread_preflight_names_missing_member(tmp_path):
    env = {**ENV, "GXANA_OUTPUT": str(tmp_path)}
    missing = st.preflight(_cfg(), "spread", ["accidentals"], env)
    assert any("variants/weighted_data/acc_weight" in m and "--steps fit,qvalue,weight" in m for m in missing)


def test_sfactor_preflight_names_xsection_command(tmp_path):
    env = {**ENV, "GXANA_OUTPUT": str(tmp_path)}
    missing = st.preflight(_cfg(), "spread", ["run"], env)
    assert missing and "xsection/data/johnson" in missing[0] and "gxana run xsection" in missing[0]


def test_plot_commands_for_fit_study():
    cmds = [c.argv for c in _plan(["spread"], studies=["fit"]) if c.argv[0].endswith("gxana_syst_plot")]
    assert len(cmds) == 4
    voigt = cmds[0]
    assert voigt[voigt.index("--layout") + 1] == "grid3"
    assert voigt[voigt.index("--name") + 1] == "weighted_diffxsec_SignalFitVoigt"
    assert voigt[voigt.index("--out-dir") + 1] == f"{OUT}/fit/plots"
    assert [voigt[i + 1] for i, a in enumerate(voigt) if a == "--input"] == [
        f"{OUT}/variants/weighted_data/{l}" for l in ("johnson", "voigt", "voigt_cheby1")]
    assert [voigt[i + 1] for i, a in enumerate(voigt) if a == "--legend"] == [
        "Nominal Fit|f", "Voigt+Cheby2|lep", "Voigt+Cheby1|lep"]
    assert voigt[voigt.index("--first-style") + 1] == "band"
    allfits = cmds[3]
    assert allfits[allfits.index("--band") + 1] == f"{OUT}/fit/fit_variations_stats.txt"


def test_plots_follow_the_stats_command():
    argvs = [c.argv for c in _plan(["spread"], studies=["accidentals"])]
    i_stats = next(i for i, a in enumerate(argvs) if "gxana_systematics.spread" in a)
    i_plot = next(i for i, a in enumerate(argvs) if a[0].endswith("gxana_syst_plot"))
    assert i_stats < i_plot


def test_track_step_runs_app_then_numbers():
    cmds = [c.argv for c in _plan(["track"])]
    assert cmds[0][0].endswith("gxana_syst_track")
    a = cmds[0]
    assert a[a.index("--out-dir") + 1] == f"{OUT}/track"
    periods = [a[i + 1] for i, x in enumerate(a) if x == "--period"]
    assert periods[0] == ("2017-01:/o/kpkpxim/qfactors/kpkpxim__M23_2017-01_ana56_nominal_kphighrap_1111111/"
                          "postQVal_flatTree_kpkpxim__M23_2017-01_ana56_nominal_kphighrap_1111111.root:"
                          "/d/flatTrees/flatTree_kpkpxim__M23_2017-01_ana56_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root:"
                          "/d/flatTrees/flatTree_thrown_kpkpxim__M23_2017-01_ana56_gen_amp_V2_ac_YstarRest.root")
    assert [a[i + 1].split(":")[0] for i, x in enumerate(a) if x == "--particle"] == ["kp1", "kp2", "pim1", "pim2", "proton"]
    n = cmds[1]
    assert n[2] == "gxana_systematics.track"
    assert n[n.index("--override") + 1] == "proton=0.05" and n[n.index("--report") + 1] == "mc"


def test_track_step_skips_other_studies():
    assert _plan(["track"], studies=["accidentals"]) == []


def test_track_preflight_names_missing_trees(tmp_path):
    env = {**ENV, "GXANA_OUTPUT": str(tmp_path / "o"), "GXANA_DATA": str(tmp_path / "d")}
    missing = st.preflight(_cfg(), "track", ["track"], env)
    assert len(missing) == 9
    assert "postQVal_flatTree_kpkpxim__M23_2017-01_ana56" in missing[0] and "gxana run qfactors" in missing[0]
    assert "flatTree_thrown_" in missing[2] and "$GXANA_DATA/flatTrees" in missing[2]
    assert st.preflight(_cfg(), "track", ["accidentals"], env) == []


def test_track_run_makes_the_study_dir(tmp_path, monkeypatch):
    monkeypatch.setattr(st, "preflight", lambda *a, **k: [])
    calls = []
    env = {**ENV, "GXANA_OUTPUT": str(tmp_path)}
    assert st.run_systematics(_cfg(), ["track"], runner=lambda argv, **k: calls.append(argv),
                              environ=env, study_names=["track"]) == 0
    assert (tmp_path / "kpkpxim/systematics/track").is_dir() and len(calls) == 2


def test_summary_step():
    (cmd,) = _plan(["summary"])
    a = cmd.argv
    assert a[2] == "gxana_systematics.summary"
    assert a[a.index("--nominal-dir") + 1] == f"{XS}/weighted_data/johnson"
    assert [a[i + 1] for i, x in enumerate(a) if x == "--column"] == [
        f"run={OUT}/run/sfactor_stats.txt", f"accidentals={OUT}/accidentals/combo_variations_stats.txt",
        f"fit={OUT}/fit/fit_variations_stats.txt"]
    assert [a[i + 1] for i, x in enumerate(a) if x == "--normalization"] == [
        f"track={OUT}/track/track_efficiency.txt", "luminosity=0.05"]


def test_summary_preflight_lists_missing_inputs(tmp_path):
    env = {**ENV, "GXANA_OUTPUT": str(tmp_path)}
    missing = st.preflight(_cfg(), "summary", None, env)
    assert len(missing) == 4
    assert all("--steps spread,track" in m for m in missing)


def test_compare_skips_unconfigured_labels(capsys):
    cmds = _plan(["compare"])
    names = [c.argv[c.argv.index("--name") + 1] for c in cmds if c.argv[0].endswith("gxana_syst_plot")]
    assert "weighted_diffxsec_QValYield" in names           # qvalues and hybrid_combo are pool labels
    assert "weighted_diffxsec_oneRFBunch" not in names      # oneRfBunch not configured
    assert "weighted_diffxsec_bkgdfit" not in names
    assert "skip" in capsys.readouterr().out.lower()


def test_run_compare_numbers_then_plots():
    cmds = [c.argv for c in _plan(["compare"], studies=["run_compare"])]
    assert cmds[0][2] == "gxana_systematics.runcompare"
    assert cmds[0][cmds[0].index("--periods-dir") + 1] == f"{XS}/data/johnson"
    run_grid = cmds[1]
    assert [run_grid[i + 1] for i, a in enumerate(run_grid) if a == "--input"] == [
        f"{XS}/data/johnson/diffxsec_flatTree_{s}" for s in (
            "kpkpxim__M23_2017-01_ana56", "kpkpxim__B4_M23_2018-01_ana03", "kpkpxim__B4_M23_2018-08_ana02")]


def test_runperiod_macro_call():
    (cmd,) = _plan(["runperiod"])
    assert cmd.argv[:4] == ["root", "-l", "-b", "-q"]
    assert cmd.argv[-1] == (f'/r/analyses/kpkpxim/systematics/GetRunPeriodPctSig.C'
                            f'("johnson","{OUT}/runperiod","{XS}/data/johnson/")')


def test_runperiod_skips_with_a_note_when_not_configured(capsys):
    cfg = _cfg()
    del cfg["systematics"]["runperiod"]
    assert _plan(["runperiod"], cfg=cfg) == []
    assert "runperiod step skipped: systematics.runperiod is not configured" in capsys.readouterr().out


def test_run_compare_stats_and_band():
    cmds = [c.argv for c in _plan(["compare"], studies=["run_compare"])]
    stats = f"{OUT}/run_compare/run_comp_stddev_scaled.txt"
    assert cmds[0][cmds[0].index("--out") + 1] == stats
    assert [cmds[0][i + 1] for i, a in enumerate(cmds[0]) if a == "--energy"] == [
        f"{a}:{b}" for a, b in zip(EDGES, EDGES[1:])]
    band = cmds[2]
    assert band[band.index("--layout") + 1] == "stddev_band" and band[band.index("--band") + 1] == stats
    assert "--band" not in cmds[1]


def test_bkgd_check_uses_the_x_axis_format():
    cfg = _cfg()
    cfg["systematics"]["variants"][0]["labels"].append({"label": "bkgd", "cheby": 1, "weight": "hybrid_combo"})
    cmds = [c.argv for c in _plan(["compare"], studies=["bkgd"], cfg=cfg)]
    (bkgd,) = cmds
    assert [bkgd[i + 1] for i, a in enumerate(bkgd) if a == "--input"] == [
        f"{OUT}/variants/weighted_data/{l}" for l in ("bkgd", "hybrid_combo")]
    assert bkgd[bkgd.index("--x-axis-format") + 1] == "205" and "--axis-format" not in bkgd


def test_compare_skips_labels_without_tables_at_run_time(tmp_path, capsys):
    env = {**ENV, "GXANA_OUTPUT": str(tmp_path)}
    assert st.plan(_cfg(), ["compare"], study_names=["qval_yield"], environ=env, runtime=True) == []
    assert "has no weighted_diffxsec_emin_" in capsys.readouterr().out
    assert len(st.plan(_cfg(), ["compare"], study_names=["qval_yield"], environ=env)) == 1


def test_compare_preflight_names_missing_period_tables(tmp_path):
    env = {**ENV, "GXANA_OUTPUT": str(tmp_path)}
    missing = st.preflight(_cfg(), "compare", ["run_compare"], env)
    assert missing and "xsection/data/johnson" in missing[0] and "--steps tables" in missing[0]
    assert st.preflight(_cfg(), "compare", ["qval_yield"], env) == []


def _nominal_tables(xs_root: Path, weighted=True, periods=True):
    if weighted:
        w = xs_root / "weighted_data/johnson"
        w.mkdir(parents=True)
        (w / "weighted_diffxsec_emin_6.40_emax_7.40.txt").write_text("h\n0.2 1 0.1 0.1 1\n")
    if periods:
        d = xs_root / "data/johnson"
        d.mkdir(parents=True)
        (d / "diffxsec_flatTree_a_emin_6.40_emax_7.40.txt").write_text("h\n0.2 1 0.1 0.1\n")


@pytest.mark.parametrize("steps,studies", [
    (list(st.DEFAULT_STEPS), None),          # fits would run first without the guard
    (["fit", "weight", "spread"], ["fit"]),  # the fit study holds the nominal
    (["spread"], ["run"]),                   # sfactor
    (["runperiod"], None),
    (["summary"], None),
    (["compare"], ["run_compare"]),          # per_period of the nominal
])
def test_missing_nominal_stops_before_anything_runs(tmp_path, capsys, steps, studies):
    env = {**ENV, "GXANA_OUTPUT": str(tmp_path)}
    calls = []
    rc = st.run_systematics(_cfg(), steps, runner=lambda *a, **k: calls.append(a), environ=env, study_names=studies)
    assert rc == 1 and calls == []
    assert not (tmp_path / "kpkpxim/systematics").exists()
    err = capsys.readouterr().err
    assert err.startswith("gxana: error:")
    assert f"{tmp_path}/kpkpxim/xsection/weighted_data/johnson" in err
    assert "gxana run xsection --channel kpkpxim --steps tables,weight" in err


def test_missing_nominal_period_tables_are_named(tmp_path, capsys):
    env = {**ENV, "GXANA_OUTPUT": str(tmp_path)}
    _nominal_tables(tmp_path / "kpkpxim/xsection", periods=False)
    calls = []
    assert st.run_systematics(_cfg(), ["spread"], runner=lambda *a, **k: calls.append(a), environ=env,
                              study_names=["run"]) == 1 and calls == []
    err = capsys.readouterr().err
    assert f"{tmp_path}/kpkpxim/xsection/data/johnson" in err and "--steps tables,weight" in err


def test_nominal_guard_ignores_steps_that_do_not_read_it(tmp_path, capsys, monkeypatch):
    monkeypatch.setattr(st, "preflight", lambda *a, **k: [])
    env = {**ENV, "GXANA_OUTPUT": str(tmp_path)}
    calls = []
    # accidentals spread holds only pool labels; compare without per_period reads the pool
    assert st.run_systematics(_cfg(), ["spread"], runner=lambda argv, **k: calls.append(argv),
                              environ=env, study_names=["accidentals"]) == 0 and calls
    assert "gxana: error:" not in capsys.readouterr().err


def test_nominal_guard_passes_when_tables_exist(tmp_path, monkeypatch):
    monkeypatch.setattr(st, "preflight", lambda *a, **k: [])
    env = {**ENV, "GXANA_OUTPUT": str(tmp_path)}
    _nominal_tables(tmp_path / "kpkpxim/xsection")
    calls = []
    assert st.run_systematics(_cfg(), ["spread"], runner=lambda argv, **k: calls.append(argv),
                              environ=env, study_names=["run"]) == 0
    assert len(calls) == 1


def test_summary_skipped_when_study_filter_excludes_its_inputs(tmp_path, capsys):
    assert _plan(["summary"], studies=["fit"]) == []
    out = capsys.readouterr().out
    assert "gxana: note: summary step skipped: --study excludes run, accidentals, track, luminosity" in out
    env = {**ENV, "GXANA_OUTPUT": str(tmp_path)}
    assert st.preflight(_cfg(), "summary", ["fit"], env) == []
    calls = []
    assert st.run_systematics(_cfg(), ["summary"], runner=lambda *a, **k: calls.append(a), environ=env,
                              study_names=["fit"]) == 0 and calls == []
    assert "summary step skipped" in capsys.readouterr().out


def test_summary_runs_when_study_filter_includes_its_inputs(capsys):
    (cmd,) = _plan(["summary"], studies=["run", "accidentals", "fit", "track", "luminosity"])
    assert cmd.argv[2] == "gxana_systematics.summary"
    assert "skipped" not in capsys.readouterr().out


def test_per_period_compare_does_not_refit_the_nominal():
    chosen, groups, qvalues = st.selected(_cfg(), ["run_compare"])
    assert groups == [] and qvalues == []
    assert _plan(["fit", "qvalue", "weight"], studies=["run_compare"]) == []


def test_run_systematics_creates_the_pool_dirs_before_the_first_command(tmp_path, monkeypatch):
    class Ok:
        returncode = 0
    monkeypatch.setattr(st, "preflight", lambda *a, **k: [])
    cfg = _cfg()
    env = {**ENV, "GXANA_OUTPUT": str(tmp_path)}
    _, groups, qvalues = st.selected(cfg, None)
    expected = [Path(f"{st._pool(cfg, env, kind)}/{label}")
                for kind in ("data", "fits", "weighted_data")
                for label in [e["label"] for g in groups for e in g["labels"]] + [q["label"] for q in qvalues]]
    assert expected
    calls = []

    def runner(argv, **kwargs):
        if not calls:
            assert all(d.is_dir() for d in expected)
        calls.append(argv)
        return Ok()

    assert st.run_systematics(cfg, ["fit"], runner=runner, environ=env) == 0
    assert calls
