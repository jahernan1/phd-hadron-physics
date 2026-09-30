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
    assert xs.STEPS.index("tex") > xs.STEPS.index("qvalue")
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
    # qvalue_rescale writes into data/qvalues/ but does not create it
    assert (tmp_path / "kpkpxim" / "xsection" / "data" / "qvalues").is_dir()


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


# fitfigs: dissertation fit-variation figures and fit_variations_stats.txt

FIT_LABELS = ["hybrid_combo", "johnson", "qvalues", "johnson_cheby1", "voigt", "voigt_cheby1",
              "mcPdf", "mcPdf_cheby1"]
EXAMPLE_PDF = "data_flatTree_kpkpxim__B4_M23_2018-08_ana02_emin_8.45_emax_8.68.pdf"
EXAMPLES = {"johnsonFit": "johnson", "voigtFit": "voigt", "mcFit": "mcPdf",
            "johnsonChebFit": "johnson_cheby1", "voigtChebFit": "voigt_cheby1", "mcChebFit": "mcPdf_cheby1"}


def test_fitfigs_is_opt_in_and_precedes_tex():
    assert "fitfigs" in xs.STEPS
    assert "fitfigs" not in xs.DEFAULT_STEPS
    # tex reads the fit_variations_stats.txt that fitfigs writes
    assert xs.STEPS.index("qvalue") < xs.STEPS.index("fitfigs") < xs.STEPS.index("tex")


def test_fitfigs_plan_weights_qvalues_first():
    cmds = _plan(["fitfigs"])
    assert all(c.step == "fitfigs" for c in cmds)
    qv = [c.argv for c in cmds if "gxana_xsection.weighted_average" in c.argv]
    assert [a[1:] for a in qv] == [
        ["-m", "gxana_xsection.weighted_average", "/o/kpkpxim/xsection/data/qvalues",
         "/o/kpkpxim/xsection/weighted_data/qvalues", "--pattern", f"diffxsec*_emin_{e}*.txt"]
        for e in ["6.40", "7.40", "7.86", "8.19", "8.45", "8.68", "9.26", "10.18"]]
    assert cmds[0].argv == qv[0]


def test_fitfigs_plan_graph_conversions_then_macro():
    cmds = [c for c in _plan(["fitfigs"]) if c.argv[0] == "root"]
    assert len(cmds) == 9
    graphs, macro = cmds[:8], cmds[8]
    for label, c in zip(FIT_LABELS, graphs):
        assert c.argv == [
            "root", "-l", "-b", "-q", "/r/rootlogon.C",
            f'/r/analyses/kpkpxim/xsection/MakeWeightedDiffXSecTGraphs.C('
            f'"/o/kpkpxim/xsection/weighted_data/{label}/",'
            f'"/o/kpkpxim/systematics/comparisons/WeightedDiffXSecTGraphs_{label}.root")']
        assert c.cwd is None
    assert macro.argv == ["root", "-l", "-b", "-q", "/r/rootlogon.C",
                          "/r/analyses/kpkpxim/systematics/comparisons/PlotFitComparison.C"]
    # the macro opens WeightedDiffXSecTGraphs_<label>.root and writes
    # fit_variations_stats.txt relative to the current directory
    assert macro.cwd == "/o/kpkpxim/systematics/comparisons"
    xcfg = config.load_channel("kpkpxim")["xsection"]
    assert "/o/kpkpxim/systematics/comparisons/fit_variations_stats.txt" in xcfg["tex"]["additional"][0].replace(
        "${GXANA_OUTPUT}", "/o")


def test_fitfigs_plan_copies_six_example_fits_last():
    cmds = _plan(["fitfigs"])
    copies = [c.argv for c in cmds[-6:]]
    assert copies == [
        ["cp", f"/o/kpkpxim/xsection/fits/{label}/{EXAMPLE_PDF}",
         f"/o/kpkpxim/xsection/plots/fit_examples/{name}.pdf"]
        for name, label in EXAMPLES.items()]
    assert cmds[-7].argv[-1].endswith("PlotFitComparison.C")


def _fitfigs_env(tmp_path):
    return {"GXANA_ROOT": "/r", "GXANA_DATA": "/d", "GXANA_OUTPUT": str(tmp_path)}


def _populate_fitfigs_inputs(tmp_path, skip=()):
    xdir = tmp_path / "kpkpxim" / "xsection"
    if "qvalues" not in skip:
        (xdir / "data" / "qvalues").mkdir(parents=True)
        (xdir / "data" / "qvalues" / "diffxsec_a_emin_6.40_emax_7.40.txt").write_text("x")
    for label in FIT_LABELS:
        if label == "qvalues" or label in skip:
            continue
        d = xdir / "weighted_data" / label
        d.mkdir(parents=True)
        (d / "weighted_diffxsec_emin_6.40_emax_7.40.txt").write_text("x")
    if "fits" not in skip:
        for label in EXAMPLES.values():
            d = xdir / "fits" / label
            d.mkdir(parents=True)
            (d / EXAMPLE_PDF).write_text("x")


def _run_fitfigs(tmp_path):
    calls = []
    rc = xs.run_xsection(config.load_channel("kpkpxim"), ["fitfigs"], dry_run=False,
                         runner=lambda *a, **k: calls.append((a, k)), environ=_fitfigs_env(tmp_path))
    return rc, calls


def test_run_fitfigs_runs_all_with_macro_cwd(tmp_path):
    _populate_fitfigs_inputs(tmp_path)
    rc, calls = _run_fitfigs(tmp_path)
    assert rc == 0
    assert len(calls) == 8 + 8 + 1 + 6
    comparisons = tmp_path / "kpkpxim" / "systematics" / "comparisons"
    macro = next((a, k) for a, k in calls if a[0][-1].endswith("PlotFitComparison.C"))
    assert macro[1]["cwd"] == str(comparisons)
    assert comparisons.is_dir()
    assert (tmp_path / "kpkpxim" / "xsection" / "plots" / "fit_examples").is_dir()
    assert (tmp_path / "kpkpxim" / "xsection" / "weighted_data" / "qvalues").is_dir()


def test_run_fitfigs_missing_qvalues_names_qvalue_step(tmp_path, capsys):
    _populate_fitfigs_inputs(tmp_path, skip=("qvalues",))
    rc, calls = _run_fitfigs(tmp_path)
    out = capsys.readouterr().out
    assert rc == 1 and calls == []
    assert "qvalues" in out and "gxana run xsection --steps qvalue" in out


def test_run_fitfigs_missing_label_names_label_and_tables_weight(tmp_path, capsys):
    _populate_fitfigs_inputs(tmp_path, skip=("voigt_cheby1",))
    rc, calls = _run_fitfigs(tmp_path)
    out = capsys.readouterr().out
    assert rc == 1 and calls == []
    assert "voigt_cheby1" in out and "gxana run xsection --steps tables,weight" in out


def test_run_fitfigs_missing_example_fit_pdf_is_loud(tmp_path, capsys):
    _populate_fitfigs_inputs(tmp_path, skip=("fits",))
    rc, calls = _run_fitfigs(tmp_path)
    out = capsys.readouterr().out
    assert rc == 1 and calls == []
    assert EXAMPLE_PDF in out and "--steps tables" in out


def test_run_fitfigs_rejects_other_diffxsec_tables_in_label_dir(tmp_path, capsys):
    # tex writes syst_weighted_diffxsec_*.txt into weighted_data/johnson; the
    # graph macro converts every *diffxsec*.txt, so a fitfigs rerun would
    # double the graphs and PlotFitComparison.C would crash.
    _populate_fitfigs_inputs(tmp_path)
    johnson = tmp_path / "kpkpxim" / "xsection" / "weighted_data" / "johnson"
    (johnson / "syst_weighted_diffxsec_emin_6.40_emax_7.40.txt").write_text("x")
    (johnson / "processed_weighted_diffxsec_emin_6.40_emax_7.40.txt").write_text("x")
    rc, calls = _run_fitfigs(tmp_path)
    out = capsys.readouterr().out
    assert rc == 1 and calls == []
    assert "syst_weighted_diffxsec_emin_6.40_emax_7.40.txt" in out and str(johnson) in out
    assert "processed_weighted_diffxsec_emin_6.40_emax_7.40.txt" in out
    assert "weighted_diffxsec_*" in out and "before tex" in out and "remove the files listed" in out


def test_fitfigs_dry_run_prints_commands(capsys):
    calls = []
    rc = xs.run_xsection(config.load_channel("kpkpxim"), ["fitfigs"], dry_run=True,
                         runner=lambda *a, **k: calls.append(a), environ=ENV)
    out = capsys.readouterr().out
    assert rc == 0 and calls == []
    assert "MakeWeightedDiffXSecTGraphs.C" in out and "PlotFitComparison.C" in out
    assert "(cd /o/kpkpxim/systematics/comparisons" in out
    assert "fit_examples/johnsonFit.pdf" in out
