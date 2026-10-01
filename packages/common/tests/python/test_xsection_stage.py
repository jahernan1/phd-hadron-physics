from pathlib import Path

import pytest

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
    assert [c[c.index("--fit") + 1] for c in cmds] == ["Johnson"]
    j = cmds[0]
    labels = [j[i + 1] for i, a in enumerate(j) if a == "--label"]
    assert labels == ["johnson"]
    chebys = [j[i + 1] for i, a in enumerate(j) if a == "--cheby"]
    assert chebys == ["2"]
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
    assert xs.STEPS == ("bin", "tables", "weight", "integrate", "components", "tex")
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
    known = r"\['bin', 'tables', 'weight', 'integrate', 'components', 'tex'\]"
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


def test_run_xsection_creates_the_output_dirs_before_the_first_command(tmp_path):
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
