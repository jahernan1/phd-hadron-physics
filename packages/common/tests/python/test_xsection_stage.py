from pathlib import Path
from types import SimpleNamespace

import pytest

from gxana import config
from gxana.paths import repo_root
from gxana.stages import xsection as xs

ENV = {"GXANA_ROOT": "/r", "GXANA_DATA": "/d", "GXANA_OUTPUT": "/o", "GXANA_ANALYSIS_DATA": "/a"}


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
    assert [c[c.index("--fit") + 1] for c in cmds] == ["Johnson"]
    j = cmds[0]
    labels = [j[i + 1] for i, a in enumerate(j) if a == "--label"]
    assert labels == ["johnson"]
    chebys = [j[i + 1] for i, a in enumerate(j) if a == "--cheby"]
    assert chebys == ["2"]
    jobs = [a for a in j if a.startswith("flatTree_")]
    assert [s.split(":")[0] for s in jobs[:3]] == [
        "flatTree_kpkpxim__M23_2017-01_ana56", "flatTree_kpkpxim__B4_M23_2018-01_ana03", "flatTree_kpkpxim__B4_M23_2018-08_ana02"]
    assert jobs[0].endswith(":/a/kpkpxim/flux/flux_30274_31057_r4.root")


def _tables_label_dirs(argv):
    """Where gxana_xsec_tables writes each --label: <--out>/<label>/."""
    out = argv[argv.index("--out") + 1]
    return [f"{out}/{argv[i + 1]}" for i, a in enumerate(argv) if a == "--label"]


def test_tables_writes_one_dir_per_label_read_by_weight_and_components():
    written = [d for c in _plan(["tables"]) for d in _tables_label_dirs(c.argv)]
    assert len(written) == len(set(written))  # no two labels share a directory
    assert "/o/kpkpxim/xsection/data/johnson" in written
    xcfg = config.load_channel("kpkpxim")["xsection"]
    weight_in = {c.argv[3] for c in _plan(["weight"])}  # argv: py -m mod DIR OUT --pattern P ...
    comp_in = {c.argv[3] for c in _plan(["components"])}
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


def test_weight_patterns():
    cmds = [c.argv for c in _plan(["weight"])]
    johnson = [c for c in cmds if c[3].endswith("/data/johnson")]  # argv: py -m mod DIR OUT --pattern P ...
    pats = [c[c.index("--pattern") + 1] for c in johnson]
    assert all(c[-2:] == ["--n-periods", "3"] for c in johnson)
    assert pats[0] == "totxsec*.txt"
    assert pats[1:] == [f"diffxsec*_emin_{e}*.txt" for e in ["6.40", "7.40", "7.86", "8.19", "8.45", "8.68", "9.26", "10.18"]]


def test_default_steps_exclude_tex():
    assert xs.STEPS == ("bin", "tables", "weight", "integrate", "components", "tex", "figures")
    assert xs.DEFAULT_STEPS == ("bin", "tables", "weight", "integrate", "components")
    assert "tex" not in xs.DEFAULT_STEPS


def test_unknown_step_rejected():
    import pytest
    with pytest.raises(config.ConfigError, match="unknown step"):
        _plan(["plot"])


def test_missing_env_names_variable():
    import pytest
    from gxana.paths import MissingEnvError
    with pytest.raises(MissingEnvError, match="GXANA_OUTPUT"):
        xs.plan_xsection(config.load_channel("kpkpxim"), ["bin"], environ={"GXANA_DATA": "/d"})


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
                          f"/o/kpkpxim/xsection/weighted_data/{label}", "--pattern", "intxsec*.txt",
                          "--n-periods", "3"]


def test_tex_plan_passes_columns_in_order():
    (cmd,) = _plan(["tex"])
    argv = cmd.argv
    assert argv[argv.index("--run-fraction") + 1] == "0.051"
    assert [argv[i + 1] for i, a in enumerate(argv) if a == "--column"] == [
        "Run Combination=/o/kpkpxim/systematics/run/sfactor_stats.txt",
        "Accidentals=/o/kpkpxim/systematics/accidentals/combo_variations_stats.txt",
        "Yield Extraction=/o/kpkpxim/systematics/fit/fit_variations_stats.txt"]


def test_qvalue_and_fitfigs_moved_out():
    assert "qvalue" not in xs.STEPS and "fitfigs" not in xs.STEPS
    xcfg = config.load_channel("kpkpxim")["xsection"]
    assert [e["label"] for f in xcfg["fits"] for e in f["labels"]] == ["johnson"]
    assert xcfg["weighted_labels"] == ["johnson"] == xcfg["component_labels"]


def _tex_env(tmp_path):
    return {"GXANA_ROOT": "/r", "GXANA_DATA": "/d", "GXANA_OUTPUT": str(tmp_path)}


def _column_files(tmp_path):
    columns = config.load_channel("kpkpxim")["xsection"]["tex"]["columns"]
    return [Path(c.replace("${GXANA_OUTPUT}", str(tmp_path))) for c in columns.values()]


def test_run_xsection_tex_missing_columns_is_loud_failure(tmp_path, capsys):
    cfg = config.load_channel("kpkpxim")
    calls = []
    rc = xs.run_xsection(cfg, ["tex"], dry_run=False, runner=lambda *a, **k: calls.append(a),
                         environ=_tex_env(tmp_path))
    out = capsys.readouterr().out
    assert rc == 1
    assert calls == []
    assert "sfactor_stats.txt" in out and "fit_variations_stats.txt" in out
    assert "gxana run systematics" in out


def test_run_xsection_tex_runs_when_columns_present(tmp_path):
    cfg = config.load_channel("kpkpxim")
    for f in _column_files(tmp_path):
        f.parent.mkdir(parents=True, exist_ok=True)
        f.write_text("x")
    calls = []
    rc = xs.run_xsection(cfg, ["tex"], dry_run=False, runner=lambda *a, **k: calls.append(a),
                         environ=_tex_env(tmp_path))
    assert rc == 0
    assert len(calls) == 1
    assert (tmp_path / "kpkpxim" / "xsection" / "tables").is_dir()


def test_tables_write_fit_plots_after_out():
    for c in _plan(["tables"]):
        i = c.argv.index("--out")
        assert c.argv[i + 2:i + 4] == ["--plots", "/o/kpkpxim/xsection/fits"]


def test_tables_without_fit_plots_emit_no_plots_option():
    cfg = config.load_channel("kpkpxim")
    del cfg["xsection"]["fit_plots"]
    assert all("--plots" not in c.argv for c in xs.plan_xsection(cfg, ["tables"], environ=ENV))


def test_unknown_step_message_names_the_alphabetically_first():
    import pytest
    known = r"\['bin', 'tables', 'weight', 'integrate', 'components', 'tex', 'figures'\]"
    with pytest.raises(config.ConfigError, match=rf"^unknown step 'plot'; known: {known}$"):
        _plan(["zzz", "plot"])
    with pytest.raises(config.ConfigError, match=rf"^unknown step 'plot'; known: {known}$"):
        xs.run_xsection(config.load_channel("kpkpxim"), ["zzz", "plot"], dry_run=True, environ=ENV)


def test_non_contiguous_t_bins_are_a_config_error():
    import pytest
    cfg = config.load_channel("kpkpxim")
    cfg["t_bins"] = [[0.1, 0.35], [0.4, 0.53]]
    with pytest.raises(config.ConfigError, match=r"^t_bins are not contiguous: \[\[0\.1, 0\.35\], \[0\.4, 0\.53\]\]$"):
        xs.plan_xsection(cfg, ["bin"], environ=ENV)


def test_weighted_average_commands_patterns():
    import sys
    cmds = xs.weighted_average_commands("/i", "/w", [6.4, 7.855, 11.4], "weight", n_periods=3, tag="vary_a_1")
    assert [c.argv for c in cmds] == [
        [sys.executable, "-m", "gxana_xsection.weighted_average", "/i", "/w", "--pattern", p, "--n-periods", "3"]
        for p in ("totxsec*_vary_a_1.txt", "diffxsec*_vary_a_1_emin_6.40*.txt", "diffxsec*_vary_a_1_emin_7.86*.txt")]
    assert all(c.step == "weight" and c.cwd is None for c in cmds)
    plain = xs.weighted_average_commands("/i", "/w", [6.4, 7.4], "s", n_periods=2, total=False)
    assert [c.argv[6:] for c in plain] == [["diffxsec*_emin_6.40*.txt", "--n-periods", "2"]] and plain[0].step == "s"


def test_run_xsection_creates_the_output_dirs_before_the_first_command(tmp_path, monkeypatch):
    monkeypatch.setattr(xs, "preflight", lambda *a, **k: [])
    cfg = config.load_channel("kpkpxim")
    expected = xs._output_dirs(cfg, _tex_env(tmp_path))
    assert expected
    calls = []

    def runner(argv, **kwargs):
        if not calls:
            assert all(d.is_dir() for d in expected)
        calls.append(argv)

    assert xs.run_xsection(cfg, ["bin"], runner=runner, environ=_tex_env(tmp_path)) == 0
    assert calls


# Channel physics keys are checked where the stages read them: a misspelt key must not be
# silently dropped (the app would then use its kpkpxim default), a half-written block must
# not be a bare KeyError.
def _channel_with(path, value):
    import copy
    cfg = copy.deepcopy(config.load_channel("kpkpxim"))
    node = cfg
    for key in path[:-1]:
        node = node.setdefault(key, {})
    if value is KeyError:
        node.pop(path[-1], None)
    else:
        node[path[-1]] = value
    return cfg


def _all_physics_args(cfg):
    xs.tables_physics_args(cfg)
    for mode in ("data", "mc", "thrown"):
        xs.bin_physics_args(cfg, mode)


UNKNOWN_KEYS = [
    (("xsection", "mass_windows", "lo_edge"), 1.0, "xsection.mass_windows.lo_edge"),
    (("xsection", "target", "densty"), 1.0, "xsection.target.densty"),
    (("physics", "reaction"), "x", "physics.reaction"),
    (("physics", "observable", "name"), "x", "physics.observable.name"),
    (("physics", "branching_ratio", "err"), 0.1, "physics.branching_ratio.err"),
]
MISSING_KEYS = [
    (("physics", "observable"), {"title": "t"}, "physics.observable.branch"),
    (("physics", "observable"), {"branch": "b"}, "physics.observable.title"),
    (("physics", "branching_ratio"), {"value": 0.6}, "physics.branching_ratio.error"),
    (("physics", "branching_ratio"), {"error": 0.1}, "physics.branching_ratio.value"),
    (("xsection", "target"), {"density": 1.0, "molar_mass": 2.0, "atoms": 2}, "xsection.target.z"),
    (("xsection", "target"), {"z": [50.4, 79.1], "molar_mass": 2.0, "atoms": 2}, "xsection.target.density"),
    (("xsection", "target"), {"z": [79.1, 50.4], "density": 1.0, "molar_mass": 2.0, "atoms": 2}, "zmin < zmax"),
]


@pytest.mark.parametrize("path, value, message", UNKNOWN_KEYS)
def test_unknown_physics_keys_are_rejected(path, value, message):
    with pytest.raises(config.ConfigError, match=message):
        _all_physics_args(_channel_with(path, value))


@pytest.mark.parametrize("path, value, message", MISSING_KEYS)
def test_incomplete_physics_blocks_are_rejected(path, value, message):
    with pytest.raises(config.ConfigError, match=message):
        _all_physics_args(_channel_with(path, value))


@pytest.mark.parametrize("gate", ["", "   ", 1.3, None, ["hybrid_combo"]])
def test_the_fit_gate_must_be_non_empty_text(gate):
    with pytest.raises(config.ConfigError, match="xsection.gate"):
        xs.tables_physics_args(_channel_with(("xsection", "gate"), gate))


# The kpkpxim channel values as the stage writes them: the text of the legacy C++ literals
# (test_xsection.cxx checks that std::stod gives back the literal's double).
KPKPXIM_TABLES_TAIL = [
    "--observable", "decayxim_M", "--observable-title", "M(#Lambda#pi^{-}) (GeV/c^{2})",
    "--gate", "(hybrid_combo)*(decayxim_M>1.3&&decayxim_M<1.35)", "--qvalue-branch", "qvalue_decayxim_M",
    "--br", "0.641,0.005", "--target", "50.4,79.1,0.07008,2.01588,2",
    "--mass-window", "lo=1.27", "--mass-window", "mc_hi=1.4", "--mass-window", "mc_signal_hi=1.38",
    "--mass-window", "mc_plot_hi=1.42", "--mass-window", "data_hi=1.45", "--mass-window", "data_edge=1.28",
    "--mass-window", "mcpdf_data_lo=1.275"]
KPKPXIM_BRANCHES = [
    "beam_E", "chisqndf", "total_mm2", "xim_pathlensig", "lambda_pathlensig", "kphigh_p4", "kplow_p4",
    "beam_vertexZ", "hybrid_combo", "kphigh_prapidity", "kplow_prapidity", "beam_E_Truth", "beam_p4_truth",
    "decayxim_M", "xim_costheta_gen_amp", "xim_costheta_hf", "t_dist", "t_dist_truth", "confidencelvl",
    "best_combo", "best_combo_rf", "acc_weight", "xim_lifetime_restframe", "lambda_lifetime_restframe",
    "decayxim_p4", "ystar_p4"]


def test_tables_end_with_the_kpkpxim_physics():
    (argv,) = [c.argv for c in _plan(["tables"])]
    assert argv[-len(KPKPXIM_TABLES_TAIL):] == KPKPXIM_TABLES_TAIL
    assert argv[-len(KPKPXIM_TABLES_TAIL) - 1].startswith("flatTree_kpkpxim__B4_M23_2018-08_ana02:")


def test_bin_ends_with_the_kpkpxim_tree_and_branches():
    cmds = [c.argv for c in _plan(["bin"])]
    branches = [a for b in KPKPXIM_BRANCHES for a in ("--branch", b)]
    assert cmds[0][8:] == ["--tree", "flatTree_kpkpxim"] + branches + ["--data-branch", "qvalue_decayxim_M"]
    assert cmds[1][8:] == ["--tree", "flatTree_kpkpxim"] + branches
    assert cmds[2][8:] == ["--tree", "flatTree_thrown_kpkpxim"]


def test_components_anchor_is_the_reaction():
    assert all(c.argv[-2:] == ["--anchor", "kpkpxim"] for c in _plan(["components"]))


def test_tables_physics_args_reject_a_reversed_target():
    import pytest
    cfg = config.load_channel("kpkpxim")
    cfg["xsection"]["target"]["z"] = [79.1, 50.4]
    with pytest.raises(config.ConfigError, match="zmin < zmax"):
        xs.tables_physics_args(cfg)
    cfg = config.load_channel("kpkpxim")
    del cfg["xsection"]["mass_windows"]["data_edge"]
    with pytest.raises(config.ConfigError, match="mass_windows"):
        xs.tables_physics_args(cfg)


def _env(tmp_path):
    return {"GXANA_ROOT": "/r", "GXANA_DATA": str(tmp_path / "d"), "GXANA_OUTPUT": str(tmp_path / "o"),
            "GXANA_ANALYSIS_DATA": str(tmp_path / "a")}


def test_preflight_bin_names_producers(tmp_path):
    missing = xs.preflight(config.load_channel("kpkpxim"), "bin", _env(tmp_path))
    assert len(missing) == 9
    assert "postQVal_flatTree_kpkpxim__M23_2017-01_ana56" in missing[0]
    assert "gxana run qfactors --channel kpkpxim --period 2017-01" in missing[0]
    assert "flatTreePrep.C" in missing[1]
    assert "gxana run select --channel kpkpxim --period 2017-01 --sample gen_amp_V2_ac_YstarRest --thrown" in missing[2]
    assert all("gxana data stage --channel kpkpxim" in m for m in missing)


def test_preflight_tables_names_binned_trees_and_flux(tmp_path):
    missing = xs.preflight(config.load_channel("kpkpxim"), "tables", _env(tmp_path))
    assert any("binned_thrown_flatTree_" in m and "--steps bin" in m for m in missing)
    assert any("flux_30274_31057_r4.root" in m and "xsection.inputs.flux_dir" in m for m in missing)


def test_preflight_weight_needs_tables(tmp_path):
    missing = xs.preflight(config.load_channel("kpkpxim"), "weight", _env(tmp_path))
    assert missing == [f"{tmp_path}/o/kpkpxim/xsection/data/johnson/diffxsec*.txt "
                       "(gxana run xsection --channel kpkpxim --steps tables)"]


def test_run_xsection_missing_inputs_runs_nothing(tmp_path, capsys):
    calls = []
    rc = xs.run_xsection(config.load_channel("kpkpxim"), ["bin"], runner=lambda *a, **k: calls.append(a),
                         environ=_env(tmp_path))
    assert rc == 1 and calls == []
    assert "gxana: error: bin: missing inputs:" in capsys.readouterr().err


def test_dry_run_skips_preflight(tmp_path):
    assert xs.run_xsection(config.load_channel("kpkpxim"), ["bin", "tables"], dry_run=True,
                           environ=_env(tmp_path)) == 0


def test_weight_precheck_sees_files_tables_wrote_in_the_same_run(tmp_path, monkeypatch):
    cfg = config.load_channel("kpkpxim")
    env = _env(tmp_path)
    xcfg, output_dir = xs._resolve_xcfg(cfg, env)
    real_preflight = xs.preflight
    monkeypatch.setattr(xs, "preflight", lambda c, step, environ=None: [] if step == "tables"
                        else real_preflight(c, step, environ))
    assert real_preflight(cfg, "weight", env)       # nothing there before the run
    ran = []

    def runner(argv, **kwargs):
        if not ran:                                  # the first command is `tables`: it writes its outputs
            for label in xcfg["weighted_labels"]:
                d = Path(xs.tables_label_dir(output_dir, label))
                d.mkdir(parents=True, exist_ok=True)
                (d / "diffxsec_x.txt").write_text("x")
        ran.append(argv)
        return SimpleNamespace(returncode=0)

    assert xs.run_xsection(cfg, ["tables", "weight"], runner=runner, environ=env) == 0
    assert len(ran) > 1


def test_preflight_components_needs_totout_and_diffout(tmp_path):
    cfg = config.load_channel("kpkpxim")
    xcfg, output_dir = xs._resolve_xcfg(cfg, _env(tmp_path))
    d = Path(xs.tables_label_dir(output_dir, xcfg["component_labels"][0]))
    d.mkdir(parents=True)
    (d / "diffxsec_x.txt").write_text("x")
    missing = xs.preflight(cfg, "components", _env(tmp_path))
    assert any(m.startswith(f"{d}/diffout*.txt") for m in missing)
    assert any(m.startswith(f"{d}/totout*.txt") for m in missing)


def test_figures_is_opt_in_and_last():
    assert xs.STEPS[-1] == "figures" and "figures" not in xs.DEFAULT_STEPS


def test_figures_plan_systematic_tables_then_macros():
    cmds = _plan(["figures"])
    syst, diff, total = cmds
    assert syst.argv[1:4] == ["-m", "gxana_xsection.syst_tables", "/o/kpkpxim/xsection/weighted_data/johnson"]
    assert [syst.argv[i + 1] for i, a in enumerate(syst.argv) if a == "--column"] == [
        "Run Combination=/o/kpkpxim/systematics/run/sfactor_stats.txt",
        "Accidentals=/o/kpkpxim/systematics/accidentals/combo_variations_stats.txt",
        "Yield Extraction=/o/kpkpxim/systematics/fit/fit_variations_stats.txt"]
    assert diff.argv == ["root", "-l", "-b", "-q", "/r/rootlogon.C",
                         '/r/analyses/kpkpxim/xsection/PlotDiffXSec.C("/o/kpkpxim/xsection","johnson",'
                         '"/o/kpkpxim/xsection/figures")']
    assert total.argv[-1] == ('/r/analyses/kpkpxim/xsection/PlotTotXsecWithClas.C('
                              '"/o/kpkpxim/systematics/variants","hybrid_combo","/o/kpkpxim/xsection/figures")')
    assert {c.step for c in cmds} == {"figures"}
    assert syst.cwd is None and diff.cwd == total.cwd == "/o/kpkpxim/xsection/figures"


def test_figures_columns_override_keeps_scale_factor():
    cfg = config.load_channel("kpkpxim")
    cfg["xsection"]["figures"]["columns"] = {"fit": "${GXANA_OUTPUT}/f.txt", "run": "scale_factor"}
    syst = xs.plan_xsection(cfg, ["figures"], environ=ENV)[0]
    assert [syst.argv[i + 1] for i, a in enumerate(syst.argv) if a == "--column"] == ["fit=/o/f.txt", "run=scale_factor"]


def test_figures_unknown_plot_key_is_a_config_error():
    cfg = config.load_channel("kpkpxim")
    cfg["xsection"]["figures"]["plots"][0]["argz"] = []
    with pytest.raises(config.ConfigError, match=r"xsection\.figures\.plots\[0\]\.argz"):
        xs.plan_xsection(cfg, ["figures"], environ=ENV)


@pytest.mark.parametrize("columns", [{}, ["a"], {"fit": 3}])
def test_figures_columns_must_be_a_mapping_of_files(columns):
    cfg = config.load_channel("kpkpxim")
    cfg["xsection"]["figures"]["columns"] = columns
    with pytest.raises(config.ConfigError, match=r"xsection\.figures\.columns"):
        xs.plan_xsection(cfg, ["figures"], environ=ENV)


def _figures_env(tmp_path):
    return {"GXANA_ROOT": str(repo_root()), "GXANA_DATA": "/d", "GXANA_OUTPUT": str(tmp_path)}


def _populate_figures(tmp_path):
    cfg = config.load_channel("kpkpxim")
    env = _figures_env(tmp_path)
    w = tmp_path / "kpkpxim" / "xsection" / "weighted_data" / "johnson"
    w.mkdir(parents=True)
    (w / "weighted_diffxsec_emin_6.40_emax_7.40.txt").write_text("x")
    for f in _column_files(tmp_path):
        f.parent.mkdir(parents=True, exist_ok=True)
        f.write_text("x")
    for plot in cfg["xsection"]["figures"]["plots"]:
        for req in plot.get("requires", []):
            path = Path(config.expand_env(req, env))
            if path.suffix:
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text("x")
            else:
                path.mkdir(parents=True, exist_ok=True)
    return cfg, env


def test_run_xsection_figures_missing_inputs_is_loud_failure(tmp_path, capsys):
    cfg = config.load_channel("kpkpxim")
    calls = []
    rc = xs.run_xsection(cfg, ["figures"], runner=lambda *a, **k: calls.append(a), environ=_figures_env(tmp_path))
    err = capsys.readouterr().err
    assert rc == 1 and calls == []
    assert err.startswith("gxana: error: figures: missing inputs:")
    for name in ("weighted_diffxsec", "sfactor_stats.txt", "variants/data/hybrid_combo", "totxsec_weighted_output.txt",
                 "gxana run systematics --channel kpkpxim"):
        assert name in err, name


def test_figures_dry_run_skips_the_check(tmp_path):
    assert xs.run_xsection(config.load_channel("kpkpxim"), ["figures"], dry_run=True,
                           environ=_figures_env(tmp_path)) == 0


def test_run_xsection_figures_runs_when_inputs_present(tmp_path):
    cfg, env = _populate_figures(tmp_path)
    calls = []
    rc = xs.run_xsection(cfg, ["figures"], runner=lambda *a, **k: calls.append((a, k)), environ=env)
    assert rc == 0 and len(calls) == 3
    figures = tmp_path / "kpkpxim" / "xsection" / "figures"
    assert figures.is_dir()
    assert "cwd" not in calls[0][1]
    assert calls[1][1]["cwd"] == calls[2][1]["cwd"] == str(figures)


def test_tex_precheck_does_not_look_for_a_scale_factor_file(tmp_path):
    cfg = config.load_channel("kpkpxim")
    xcfg = cfg["xsection"]
    xcfg["tex"]["columns"] = {"Run Combination": "scale_factor"}
    assert xs._tex_missing_inputs_message(cfg, xcfg, str(tmp_path), _figures_env(tmp_path)) is None


def test_with_systematics_regenerated_is_the_config_as_written():
    cfg = config.load_channel("kpkpxim")
    assert xs.with_systematics(cfg, "regenerated") is cfg


def test_with_systematics_published_reads_the_preserved_inputs():
    cfg = config.load_channel("kpkpxim")
    env = {**ENV, "GXANA_ANALYSIS_DATA": "/a"}
    pub = xs.with_systematics(cfg, "published")
    assert cfg["xsection"]["tex"]["columns"]["Accidentals"].startswith("${GXANA_OUTPUT}")   # input not modified
    tex = xs.plan_xsection(pub, ["tex"], environ=env)[0]
    tables = "/a/kpkpxim/reference/xsection/tables"
    assert [tex.argv[i + 1] for i, a in enumerate(tex.argv) if a == "--column"] == [
        "Run Combination=scale_factor",
        f"Accidentals={tables}/combo_variations_stats.txt",
        f"Yield Extraction={tables}/fit_variations_stats.txt"]
    assert tex.argv[tex.argv.index("gxana_xsection.tex_table") + 3] == \
        "/o/kpkpxim/xsection/tables/diffxsec_table_scale.tex"                               # tex.output kept
    syst, diff, total = xs.plan_xsection(pub, ["figures"], environ=env)
    assert [syst.argv[i + 1] for i, a in enumerate(syst.argv) if a == "--column"] == [
        f"fit={tables}/fit_variations_stats.txt", f"combo={tables}/combo_variations_stats.txt", "run=scale_factor"]
    assert total.argv[-1] == ('/r/analyses/kpkpxim/xsection/PlotTotXsecWithClas.C("","hybrid_combo",'
                              '"/o/kpkpxim/xsection/figures","/a/kpkpxim/reference/xsection/hybrid_combo",'
                              '"/a/kpkpxim/reference/xsection/weighted/hybrid_combo")')


def test_with_systematics_unknown_source_or_key_is_a_config_error():
    cfg = config.load_channel("kpkpxim")
    with pytest.raises(config.ConfigError, match="--systematics"):
        xs.with_systematics(cfg, "thesis")
    cfg["xsection"]["published_systematics"]["tex"]["colums"] = {}
    with pytest.raises(config.ConfigError, match=r"xsection\.published_systematics\.tex\.colums"):
        xs.with_systematics(cfg, "published")
    del cfg["xsection"]["published_systematics"]
    with pytest.raises(config.ConfigError, match=r"xsection\.published_systematics"):
        xs.with_systematics(cfg, "published")


def test_published_missing_inputs_point_at_the_preserved_data(tmp_path, capsys):
    env = {**_figures_env(tmp_path), "GXANA_ANALYSIS_DATA": str(tmp_path / "none")}
    pub = xs.with_systematics(config.load_channel("kpkpxim"), "published")
    assert xs.run_xsection(pub, ["tex"], runner=lambda *a, **k: None, environ=env) == 1
    out = capsys.readouterr().out
    assert "fit_variations_stats.txt (preserved data, gxana data status --channel kpkpxim)" in out
    assert "gxana run systematics" not in out
    missing = xs.preflight(pub, "figures", env)
    assert any("weighted/hybrid_combo/totxsec_weighted_output.txt (preserved data" in m for m in missing)
    assert not any("gxana run systematics" in m for m in missing)
