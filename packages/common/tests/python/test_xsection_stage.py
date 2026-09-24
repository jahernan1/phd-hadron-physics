from gxana import config
from gxana.stages import xsection as xs

ENV = {"GXANA_ROOT": "/r", "GXANA_DATA": "/d", "GXANA_OUTPUT": "/o"}


def _plan(steps):
    return xs.plan_xsection(config.load_channel("kpkpxim"), steps, environ=ENV)


def test_bin_commands_period_order_and_names():
    cmds = [c.argv for c in _plan(["bin"])]
    assert len(cmds) == 9
    assert cmds[0][1:4] == ["data",
        "/o/kpkpxim/qfactors/kpkpxim__M23_2017-01_ana56_nominal_kphighrap_1111111/postQVal_flatTree_kpkpxim__M23_2017-01_ana56_nominal_kphighrap_1111111.root",
        "/o/kpkpxim/xsection/binned_trees/binned_flatTree_kpkpxim__M23_2017-01_ana56_nominal_kphighrap.root"]
    assert cmds[2][1] == "thrown" and cmds[2][3].endswith("binned_thrown_flatTree_kpkpxim__M23_2017-01_ana56_gen_amp_V2_ac_YstarRest.root")
    assert "kpkpxim__B4_M23_2018-08_ana02" in cmds[6][2]
    assert cmds[0][cmds[0].index("--energy") + 1] == "6.4,7.4,7.86,8.19,8.45,8.68,9.26,10.18,11.4"


def test_tables_one_process_per_fit_labels_in_order():
    cmds = [c.argv for c in _plan(["tables"])]
    assert len(cmds) == 2
    j = cmds[0]
    assert j[j.index("--fit") + 1] == "Johnson"
    labels = [j[i + 1] for i, a in enumerate(j) if a == "--label"]
    assert labels == ["johnson", "johnson_cheby1"]
    chebys = [j[i + 1] for i, a in enumerate(j) if a == "--cheby"]
    assert chebys == ["2", "1"]
    jobs = [a for a in j if a.startswith("flatTree_")]
    assert [s.split(":")[0] for s in jobs[:3]] == [
        "flatTree_kpkpxim__M23_2017-01_ana56", "flatTree_kpkpxim__B4_M23_2018-01_ana03", "flatTree_kpkpxim__B4_M23_2018-08_ana02"]
    assert jobs[0].endswith(":/d/flux/flux_30274_31057_r4.root")


def test_tables_matches_golden_invocation():
    """The stage's Johnson/johnson job equals what test_xsec_golden runs."""
    j = _plan(["tables"])[0].argv
    params = [j[i + 1] for i, a in enumerate(j) if a == "--param"]
    assert params == ["delta=1.0,0.2,1.5", "gamma=0.0,-0.5,0.5", "lambda=0.004,0.003,0.01", "mu=1.3217,1.31,1.33"]
    assert j[j.index("--weight") + 1] == "hybrid_combo"


def test_weight_patterns():
    cmds = [c.argv for c in _plan(["weight"])]
    johnson = [c for c in cmds if c[-4].endswith("/data/johnson")]  # argv: py -m mod DIR OUT --pattern P
    pats = [c[-1] for c in johnson]
    assert pats[0] == "totxsec*.txt"
    assert pats[1:] == [f"diffxsec*_emin_{e}*.txt" for e in ["6.40", "7.40", "7.86", "8.19", "8.45", "8.68", "9.26", "10.18"]]


def test_default_steps_excludes_qvalue():
    # qvalue needs qvalue_label to point at a directory the tables step
    # actually populates (see _qvalue_source_dir); it is opt-in, not run by
    # default with the other steps.
    assert xs.DEFAULT_STEPS == ("bin", "tables", "weight", "components")
    assert "qvalue" not in xs.DEFAULT_STEPS
    assert set(xs.DEFAULT_STEPS) <= set(xs.STEPS)


def test_unknown_step_rejected():
    import pytest
    with pytest.raises(config.ConfigError, match="unknown step"):
        _plan(["plot"])


def test_missing_env_names_variable():
    import pytest
    from gxana.paths import MissingEnvError
    with pytest.raises(MissingEnvError, match="GXANA_OUTPUT"):
        xs.plan_xsection(config.load_channel("kpkpxim"), ["bin"], environ={"GXANA_DATA": "/d"})


def test_qvalue_plan_source_dir_uses_qvalue_label(tmp_path):
    cfg = config.load_channel("kpkpxim")
    cfg["xsection"]["qvalue_label"] = "johnson"
    env = {"GXANA_ROOT": "/r", "GXANA_DATA": "/d", "GXANA_OUTPUT": str(tmp_path)}
    src_dir = tmp_path / "kpkpxim" / "xsection" / "data" / "johnson"
    src_dir.mkdir(parents=True)
    (src_dir / "diffout_a.txt").write_text("x")
    (src_dir / "diffxsec_a.txt").write_text("x")
    cmds = xs.plan_xsection(cfg, ["qvalue"], environ=env)
    assert len(cmds) == 1
    argv = cmds[0].argv
    assert str(src_dir / "diffout_a.txt") in argv
    assert str(src_dir / "diffxsec_a.txt") in argv
    assert argv[-1].endswith("data/qvalues/diffxsec_a.txt")


def test_run_xsection_qvalue_missing_dir_is_loud_failure(tmp_path, capsys):
    cfg = config.load_channel("kpkpxim")
    env = {"GXANA_ROOT": "/r", "GXANA_DATA": "/d", "GXANA_OUTPUT": str(tmp_path)}
    calls = []
    rc = xs.run_xsection(cfg, ["qvalue"], dry_run=False, runner=lambda *a, **k: calls.append(a), environ=env)
    out = capsys.readouterr().out
    assert rc != 0
    assert calls == []
    assert "qvalue_label" in out
    src_dir = tmp_path / "kpkpxim" / "xsection" / "data" / "hybrid_combo"
    assert str(src_dir) in out


def test_run_xsection_qvalue_runs_when_dir_populated(tmp_path):
    cfg = config.load_channel("kpkpxim")
    env = {"GXANA_ROOT": "/r", "GXANA_DATA": "/d", "GXANA_OUTPUT": str(tmp_path)}
    src_dir = tmp_path / "kpkpxim" / "xsection" / "data" / "hybrid_combo"
    src_dir.mkdir(parents=True)
    (src_dir / "diffout_a.txt").write_text("x")
    (src_dir / "diffxsec_a.txt").write_text("x")
    calls = []
    rc = xs.run_xsection(cfg, ["qvalue"], dry_run=False, runner=lambda *a, **k: calls.append(a), environ=env)
    assert rc == 0
    assert len(calls) == 1


def test_dry_run_prints_and_runs_nothing(capsys):
    calls = []
    rc = xs.run_xsection(config.load_channel("kpkpxim"), ["tables"], dry_run=True,
                         runner=lambda *a, **k: calls.append(a), environ=ENV)
    assert rc == 0 and calls == []
    assert "gxana_xsec_tables" in capsys.readouterr().out
