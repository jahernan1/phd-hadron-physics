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


def _fit_argv(model):
    return next(c.argv for c in _plan(["tables"]) if c.argv[c.argv.index("--fit") + 1] == model)


def test_tables_one_process_per_fit_labels_in_order():
    cmds = [c.argv for c in _plan(["tables"])]
    assert [c[c.index("--fit") + 1] for c in cmds] == ["JohnsonMCShape", "Johnson", "Voigtian", "MCPdf"]
    j = cmds[1]
    labels = [j[i + 1] for i, a in enumerate(j) if a == "--label"]
    assert labels == ["johnson", "johnson_cheby1"]
    chebys = [j[i + 1] for i, a in enumerate(j) if a == "--cheby"]
    assert chebys == ["2", "1"]
    jobs = [a for a in j if a.startswith("flatTree_")]
    assert [s.split(":")[0] for s in jobs[:3]] == [
        "flatTree_kpkpxim__M23_2017-01_ana56", "flatTree_kpkpxim__B4_M23_2018-01_ana03", "flatTree_kpkpxim__B4_M23_2018-08_ana02"]
    assert jobs[0].endswith(":/d/flux/flux_30274_31057_r4.root")


def _tables_label_dirs(argv):
    """Where gxana_xsec_tables writes each --label: <--out>/<label>/."""
    out = argv[argv.index("--out") + 1]
    return [f"{out}/{argv[i + 1]}" for i, a in enumerate(argv) if a == "--label"]


def test_tables_writes_one_dir_per_label_read_by_weight_and_components():
    written = [d for c in _plan(["tables"]) for d in _tables_label_dirs(c.argv)]
    assert len(written) == len(set(written))  # no two labels share a directory
    assert "/o/kpkpxim/xsection/data/johnson" in written
    assert "/o/kpkpxim/xsection/data/johnson_cheby1" in written
    xcfg = config.load_channel("kpkpxim")["xsection"]
    weight_in = {c.argv[-4] for c in _plan(["weight"])}
    comp_in = {c.argv[-4] for c in _plan(["components"])}
    assert weight_in == {f"/o/kpkpxim/xsection/data/{l}" for l in xcfg["weighted_labels"]}
    assert comp_in == {f"/o/kpkpxim/xsection/data/{l}" for l in xcfg["component_labels"]}
    assert weight_in | comp_in <= set(written)
    assert xs.tables_label_dir("/o/kpkpxim/xsection", "johnson") == "/o/kpkpxim/xsection/data/johnson"


def test_tables_matches_golden_invocation():
    """The stage's Johnson/johnson job equals what test_xsec_golden runs."""
    j = _fit_argv("Johnson")
    params = [j[i + 1] for i, a in enumerate(j) if a == "--param"]
    assert params == ["delta=1.0,0.2,1.5", "gamma=0.0,-0.5,0.5", "lambda=0.004,0.003,0.01", "mu=1.3217,1.31,1.33"]
    assert j[j.index("--weight") + 1] == "hybrid_combo"


def _option_values(argv, option):
    return [argv[i + 1] for i, a in enumerate(argv) if a == option]


def test_tables_mcshape_fit_writes_one_label_per_combo_weight():
    """Legacy MakeXSecFiles.C: data/<accType>/ for each combo-selection weight;
    data/hybrid_combo is the JohnsonMCShape study, not the dissertation result (label johnson)."""
    j = _fit_argv("JohnsonMCShape")
    assert _option_values(j, "--param") == [
        "mu=1.3217,1.32,1.33", "lambda=0.004,0.002,0.007", "gamma=-0.01,-1.0,1.0", "delta=1.2,0.2,5.0"]
    combos = ["hybrid_combo", "best_combo", "acc_weight"]
    assert _option_values(j, "--label") == combos
    assert _option_values(j, "--weight") == combos
    assert _option_values(j, "--cheby") == ["2", "2", "2"]
    # each --weight/--label pair precedes its three period JOBs
    first_job = next(i for i, a in enumerate(j) if a.startswith("flatTree_"))
    assert j[first_job - 6:first_job] == ["--weight", "hybrid_combo", "--cheby", "2", "--label", "hybrid_combo"]
    xcfg = config.load_channel("kpkpxim")["xsection"]
    assert set(combos) <= set(xcfg["weighted_labels"]) and set(combos) <= set(xcfg["component_labels"])
    assert xcfg["qvalue_source"] == "hybrid_combo"


def test_tables_variations_use_default_weight():
    for model in ("Johnson", "Voigtian"):
        assert set(_option_values(_fit_argv(model), "--weight")) == {"hybrid_combo"}


def test_weight_patterns():
    cmds = [c.argv for c in _plan(["weight"])]
    johnson = [c for c in cmds if c[-4].endswith("/data/johnson")]  # argv: py -m mod DIR OUT --pattern P
    pats = [c[-1] for c in johnson]
    assert pats[0] == "totxsec*.txt"
    assert pats[1:] == [f"diffxsec*_emin_{e}*.txt" for e in ["6.40", "7.40", "7.86", "8.19", "8.45", "8.68", "9.26", "10.18"]]


def test_default_steps_excludes_qvalue():
    # qvalue reads the directory qvalue_source names (see _qvalue_source_dir);
    # it is opt-in, not run by default with the other steps.
    assert xs.DEFAULT_STEPS == ("bin", "tables", "weight", "integrate", "components")
    assert "qvalue" not in xs.DEFAULT_STEPS
    assert "tex" not in xs.DEFAULT_STEPS
    assert xs.STEPS.index("integrate") == xs.STEPS.index("weight") + 1
    assert xs.STEPS.index("tex") == xs.STEPS.index("qvalue") + 1
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


def test_qvalue_plan_source_dir_uses_qvalue_source(tmp_path):
    cfg = config.load_channel("kpkpxim")
    cfg["xsection"]["qvalue_source"] = "johnson"
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
    assert "qvalue_source" in out
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


def test_integrate_plan_per_label():
    cmds = [c.argv for c in _plan(["integrate"])]
    xcfg = config.load_channel("kpkpxim")["xsection"]
    assert len(cmds) == 2 * len(xcfg["weighted_labels"])
    first, second = cmds[0], cmds[1]
    label = xcfg["weighted_labels"][0]
    assert first[1:] == ["-m", "gxana_xsection.integrated_total",
                         f"/o/kpkpxim/xsection/data/{label}", f"/o/kpkpxim/xsection/data/{label}"]
    assert second[1:] == ["-m", "gxana_xsection.weighted_average", f"/o/kpkpxim/xsection/data/{label}",
                          f"/o/kpkpxim/xsection/weighted_data/{label}", "--pattern", "intxsec*.txt"]


def test_tex_plan_dissertation_tables():
    cmds = _plan(["tex"])
    assert len(cmds) == 1 and cmds[0].step == "tex"
    argv = cmds[0].argv
    assert argv[1:] == [
        "-m", "gxana_xsection.tex_table", "/o/kpkpxim/xsection/weighted_data/johnson", "weighted*.txt",
        "/o/kpkpxim/xsection/tables/diffxsec_table_scale.tex", "--systematic-source", "scale_factor",
        "--additional", "/o/kpkpxim/systematics/comparisons/fit_variations_stats.txt",
        "/o/kpkpxim/systematics/comparisons/combo_variations_stats.txt"]


def test_run_xsection_tex_missing_additional_is_loud_failure(tmp_path, capsys):
    cfg = config.load_channel("kpkpxim")
    env = {"GXANA_ROOT": "/r", "GXANA_DATA": "/d", "GXANA_OUTPUT": str(tmp_path)}
    calls = []
    rc = xs.run_xsection(cfg, ["tex"], dry_run=False, runner=lambda *a, **k: calls.append(a), environ=env)
    out = capsys.readouterr().out
    assert rc == 1
    assert calls == []
    assert "fit_variations_stats.txt" in out and "combo_variations_stats.txt" in out


def test_run_xsection_tex_runs_when_additional_present(tmp_path):
    cfg = config.load_channel("kpkpxim")
    env = {"GXANA_ROOT": "/r", "GXANA_DATA": "/d", "GXANA_OUTPUT": str(tmp_path)}
    comp = tmp_path / "kpkpxim" / "systematics" / "comparisons"
    comp.mkdir(parents=True)
    (comp / "fit_variations_stats.txt").write_text("x")
    (comp / "combo_variations_stats.txt").write_text("x")
    calls = []
    rc = xs.run_xsection(cfg, ["tex"], dry_run=False, runner=lambda *a, **k: calls.append(a), environ=env)
    assert rc == 0
    assert len(calls) == 1
    assert (tmp_path / "kpkpxim" / "xsection" / "tables").is_dir()


def test_tables_mcpdf_fit_labels_no_params():
    m = _fit_argv("MCPdf")
    assert "--param" not in m
    assert _option_values(m, "--label") == ["mcPdf", "mcPdf_cheby1"]
    assert _option_values(m, "--cheby") == ["2", "1"]
    assert _option_values(m, "--weight") == ["hybrid_combo", "hybrid_combo"]


def test_tables_write_fit_plots_after_out():
    for c in _plan(["tables"]):
        i = c.argv.index("--out")
        assert c.argv[i + 2:i + 4] == ["--plots", "/o/kpkpxim/xsection/fits"]


def test_tables_without_fit_plots_emit_no_plots_option():
    cfg = config.load_channel("kpkpxim")
    del cfg["xsection"]["fit_plots"]
    assert all("--plots" not in c.argv for c in xs.plan_xsection(cfg, ["tables"], environ=ENV))


def test_weighted_labels_include_mcpdf():
    xcfg = config.load_channel("kpkpxim")["xsection"]
    assert {"mcPdf", "mcPdf_cheby1"} <= set(xcfg["weighted_labels"])
