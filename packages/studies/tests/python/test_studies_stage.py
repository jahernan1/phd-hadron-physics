"""gxana run studies: config checks, planned argv, dry run and preflight (kpkpxim studies.yaml)."""
import copy
import re
import shlex
import subprocess
from pathlib import Path

import pytest

from gxana import cli
from gxana import config as gconfig
from gxana.paths import repo_root
from gxana_studies import config, stage

ENV = {"GXANA_ROOT": "/r", "GXANA_DATA": "/d", "GXANA_OUTPUT": "/o"}
SCANS = ["chisqndf_scan", "mm2_scan"]
STEM = "kpkpxim__M23_2017-01_ana56"
P = "/o/kpkpxim/cut_analysis_plots"


@pytest.fixture()
def cfg():
    return gconfig.load_channel("kpkpxim")


def _env(tmp_path):
    return {"GXANA_ROOT": str(tmp_path / "r"), "GXANA_DATA": str(tmp_path / "d"), "GXANA_OUTPUT": str(tmp_path / "o")}


def test_plan_order_and_count(cfg):
    cmds = stage.plan(cfg, stage.STEPS, SCANS, ENV)
    assert [c.step for c in cmds] == ["fill"] * 6 + ["fit"] + ["plot"] * 6
    assert [c.argv[:2] for c in cmds] == [["gxana_study_cutscan", c.step] for c in cmds]
    assert [c.argv[c.argv.index("--name") + 1] for c in cmds[7:]] == ["chisqndf_scan"] * 3 + ["mm2_scan"] * 3


def test_fit_is_one_command_in_the_macro_order(cfg):
    """TMinuit keeps its state between fits: one process, period outer, chisqndf then mm2 inside."""
    cmds = stage.plan(cfg, ["fit"], SCANS, ENV)
    assert len(cmds) == 1
    argv = cmds[0].argv
    assert argv[:2] == ["gxana_study_cutscan", "fit"] and argv.count("--next") == 5
    tables = [argv[i + 1] for i, a in enumerate(argv) if a == "--tables"]
    assert [t.split("/")[-1].split("_")[0] for t in tables] == ["chisqndfCut", "total"] * 3
    stems = [t.rsplit("flatTree_", 1)[1] for t in tables]
    assert stems[0] == stems[1] and stems[2] == stems[3] and stems[0] != stems[2]


def test_fill_argv(cfg):
    fill = stage.plan(cfg, ["fill"], ["chisqndf_scan"], ENV)[0].argv
    assert fill == [
        "gxana_study_cutscan", "fill", "--input", f"/d/Trees/flatTree/rawTrees/flatTree_{STEM}.root",
        "--tree", "flatTree_kpkpxim", "--out", f"{P}/data/chisqndfCut_hist_flatTree_{STEM}.root",
        "--weight", "hybrid_combo",
        "--define", "hybrid_combo=best_combo_rf*acc_weight", "--define", "kphigh_prapidity=kphigh_p4.Rapidity()",
        "--filter", "beam_E > 6.4 && beam_E < 11.4", "--filter", "abs(total_mm2) < 0.020000",
        "--filter", "beam_vertexZ > 50.4 && beam_vertexZ < 79.1", "--filter", "xim_pathlensig > 2.000000",
        "--filter", "lambda_pathlensig > 0", "--filter", "kphigh_p4.Rapidity()>2",
        "--mass", "decayxim_M:100,1.25,1.45", "--scan", "chisqndf:24,0,12",
        "--threads", "0"]
    mm2 = stage.plan(cfg, ["fill"], ["mm2_scan"], ENV)[0].argv
    assert mm2[mm2.index("--scan") + 1] == "total_mm22:24,0,0.048"
    first = mm2.index("--filter")
    assert mm2[first + 2:first + 6] == ["--define", "total_mm22=TMath::Abs(total_mm2)", "--filter", "chisqndf < 15"]


def test_fit_and_plot_argv(cfg):
    blocks = stage.plan(cfg, ["fit"], ["chisqndf_scan"], ENV)
    assert len(blocks) == 1
    argv = blocks[0].argv
    assert argv.count("--next") == 2
    fit = argv[:argv.index("--next")]
    assert fit == [
        "gxana_study_cutscan", "fit", "--hist", f"{P}/data/chisqndfCut_hist_flatTree_{STEM}.root",
        "--tables", f"{P}/data/chisqndfCut_{{what}}_flatTree_{STEM}.txt",
        "--grid-pdf", f"{P}/results/XiMinus_chisqndfCut_Fits_flatTree_{STEM}_kphighrap.pdf",
        "--first-bin", "4", "--panel-label", "#chi^{2}_{#nu}", "--mass-title", "M(#Lambda#pi^{-}) (GeV/c^{2})",
        "--range", "1.28,1.45",
        "--param", "a0=0.8,0.1,1.5", "--param", "a1=-0.2,-1,-0.1", "--param", "mu=1.3217,1.32,1.33",
        "--param", "lambda=0.0055,0.004,0.006", "--param", "gamma=0", "--param", "delta=1.3,1.,2.",
        "--param", "nbkgd=2000,1,1e6", "--param", "nxi=1000,1,1e6"]
    plot = stage.plan(cfg, ["plot"], ["mm2_scan"], ENV)[0].argv
    assert plot == [
        "gxana_study_cutscan", "plot", "--tables", f"{P}/data/total_mm2Cut_{{what}}_flatTree_{STEM}.txt",
        "--title", " ;#left|p^{#mu}_{MM_{X}}#right|^{2} #lower[0.15]{(GeV^{2} )}; N_{S}#scale[1.6]{/}#sqrt{N_{S}+N_{B}}",
        "--cut", "0.02", "--name", "mm2_scan",
        "--pdf", f"{P}/results/total_mm2Cut_flatTree_{STEM}_kphighrap.pdf",
        "--pdf", f"/o/kpkpxim/analysis/event_selection/chisqndf_cut/total_mm2Cut_flatTree_{STEM}_kphighrap.pdf"]


@pytest.mark.parametrize("edit, message", [
    (lambda s: s.update(colour="red"), "studies.chisqndf_scan.colour: unknown key"),
    (lambda s: s.pop("tree"), "studies.chisqndf_scan.tree: required"),
    (lambda s: s.update(kind="lineshape"), "studies.chisqndf_scan.kind: one of cutscan, datamc, got 'lineshape'"),
    (lambda s: s["steps"].append({"filter": "x>1", "define": "y"}), "studies.chisqndf_scan.steps[8]: need {filter: EXPR}"),
    (lambda s: s["fit"]["params"].update(mu=1.3), "studies.chisqndf_scan.fit.params.mu: need a non-empty string"),
    (lambda s: s["fit"]["params"].pop("nxi"), "studies.chisqndf_scan.fit.params.nxi: required"),
    (lambda s: s["outputs"].update(tables="data/t_{stem}.txt"), "studies.chisqndf_scan.outputs.tables: needs {what}"),
    (lambda s: s["scan"].update(first_bin=0), "studies.chisqndf_scan.scan.first_bin: need an integer >= 1"),
])
def test_config_errors(cfg, edit, message):
    cfg = copy.deepcopy(cfg)
    edit(cfg["studies"]["chisqndf_scan"])
    with pytest.raises(gconfig.ConfigError, match=re.escape(message)):
        config.validate(cfg)


def test_duplicate_outputs_are_an_error(cfg):
    cfg = copy.deepcopy(cfg)
    cfg["studies"]["mm2_scan"]["outputs"] = cfg["studies"]["chisqndf_scan"]["outputs"]
    with pytest.raises(gconfig.ConfigError,
                       match=r"studies\.mm2_scan: writes .*/data/chisqndfCut_hist_flatTree_\{stem\}\.root, "
                             r"as studies\.chisqndf_scan does"):
        config.validate(cfg)


def test_unknown_study_step_and_placeholder(cfg):
    with pytest.raises(gconfig.ConfigError, match="unknown study 'nope'"):
        stage.plan(cfg, ["fill"], ["nope"], ENV)
    with pytest.raises(gconfig.ConfigError, match="unknown step 'tables'"):
        stage.plan(cfg, ["tables"], None, ENV)
    cfg = copy.deepcopy(cfg)
    cfg["studies"]["chisqndf_scan"]["input"] = "${GXANA_DATA}/{sample}.root"
    with pytest.raises(gconfig.ConfigError, match="unknown or malformed placeholder"):
        stage.plan(cfg, ["fill"], ["chisqndf_scan"], ENV)


def test_dry_run_prints_every_command_and_runs_nothing(cfg, tmp_path, capsys):
    env = _env(tmp_path)

    def runner(*args, **kwargs):
        raise AssertionError("dry run ran a command")

    assert stage.run_studies(cfg, list(stage.STEPS), dry_run=True, runner=runner, environ=env, study_names=SCANS) == 0
    lines = capsys.readouterr().out.splitlines()
    assert len(lines) == 13
    assert lines[0] == shlex.join(stage.plan(cfg, ["fill"], SCANS, env)[0].argv)
    assert not (tmp_path / "o").exists()


def test_missing_inputs_stop_before_any_command(cfg, tmp_path, capsys):
    env = _env(tmp_path)
    calls = []
    rc = stage.run_studies(cfg, list(stage.STEPS), runner=lambda argv, **k: calls.append(argv), environ=env,
                           study_names=SCANS)
    err = capsys.readouterr().err
    assert rc == 1 and calls == []
    assert "gxana: error: fill: missing inputs:" in err
    assert f"{tmp_path}/d/Trees/flatTree/rawTrees/flatTree_{STEM}.root (input of study chisqndf_scan)" in err
    assert err.count("(input of study ") == 6
    assert (tmp_path / "o/kpkpxim/cut_analysis_plots/data").is_dir()  # output directories are made first


def test_fit_preflight_names_the_fill_command(cfg, tmp_path, capsys):
    rc = stage.run_studies(cfg, ["fit"], runner=lambda argv, **k: None, environ=_env(tmp_path), study_names=["mm2_scan"])
    err = capsys.readouterr().err
    assert rc == 1
    assert (f"{tmp_path}/o/kpkpxim/cut_analysis_plots/data/total_mm2Cut_hist_flatTree_{STEM}.root "
            "(gxana run studies --channel kpkpxim --study mm2_scan --steps fill)") in err


def test_runs_every_step_in_order(cfg, tmp_path):
    env = _env(tmp_path)
    for period in cfg["periods"]:
        tree = tmp_path / "d/Trees/flatTree/rawTrees" / f"flatTree_{gconfig.tree_stem(cfg, period, 'data')}.root"
        tree.parent.mkdir(parents=True, exist_ok=True)
        tree.write_text("")
    calls = []

    def runner(argv, check=False, **kwargs):
        calls.append(argv[1])
        if argv[1] == "fill":
            Path(argv[argv.index("--out") + 1]).write_text("")
        if argv[1] == "fit":
            for i in (i for i, a in enumerate(argv) if a == "--tables"):
                for what in ("FOM", "SB", "Yield"):
                    Path(argv[i + 1].replace("{what}", what)).write_text("")
        return subprocess.CompletedProcess(argv, 0)

    assert stage.run_studies(cfg, list(stage.STEPS), runner=runner, environ=env, study_names=SCANS) == 0
    assert calls == ["fill"] * 6 + ["fit"] + ["plot"] * 6


def test_cli_dry_run(monkeypatch, capsys):
    for key, value in {"GXANA_ROOT": str(repo_root()), "GXANA_DATA": "/d", "GXANA_OUTPUT": "/o"}.items():
        monkeypatch.setenv(key, value)
    assert cli.main(["run", "studies", "--channel", "kpkpxim", "--study", "mm2_scan", "--steps", "plot",
                     "--dry-run"]) == 0
    lines = capsys.readouterr().out.splitlines()
    assert len(lines) == 3
    assert all(line.split()[0].endswith("gxana_study_cutscan") and line.split()[1] == "plot" for line in lines)
    assert cli.main(["run", "studies", "--channel", "kpkpxim", "--study", "nope", "--dry-run"]) == 2
    assert "unknown study 'nope'" in capsys.readouterr().err


KIN = "/o/kpkpxim/data_mc_kinematics"


def test_kinematics_argv(cfg):
    fill, plot = (c.argv for c in stage.plan(cfg, stage.STEPS, ["kinematics"], ENV))
    assert fill[:8] == ["gxana_study_datamc", "fill", "--out", f"{KIN}/kinematics_kphighrap.root",
                        "--tree", "flatTree_kpkpxim", "--thrown-tree", "flatTree_thrown_kpkpxim"]
    assert fill[8:10] == ["--period", (
        f"2017-01:Spring_2017:/o/kpkpxim/qfactors/{STEM}_nominal_kphighrap_1111111/"
        f"postQVal_flatTree_{STEM}_nominal_kphighrap_1111111.root:"
        f"/d/flatTrees/flatTree_{STEM}_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root:"
        f"/d/Trees/flatTree/rawTrees/flatTree_thrown_{STEM}_gen_amp_V2_ac_YstarRest.root")]
    assert fill[14:fill.index("--var")] == [
        "--filter", "data:beam_E > 6.4 && beam_E < 11.4", "--define", "data:qvalue_acc=qvalue_decayxim_M*hybrid_combo",
        "--weight", "data:qvalue_acc", "--filter", "mc:beam_E > 6.4 && beam_E < 11.4", "--weight", "mc:hybrid_combo",
        "--filter", "thrown:beam_E > 6.4 && beam_E < 11.4", "--filter", "thrown:main_pid==1",
        "--define", "thrown:kp_highp_P3=kp1_p4.P()", "--define", "thrown:kp_lowp_P3=kp2_p4.P()",
        "--define", "thrown:pim1_P3=pim1_p4.P()", "--define", "thrown:pim2_P3=pim2_p4.P()",
        "--define", "thrown:proton_P3=proton_p4.P()"]
    assert fill.count("--var") == 29 and fill.count("--truth-var") == 7
    assert plot[:12] == ["gxana_study_datamc", "plot", "--in", f"{KIN}/kinematics_kphighrap.root", "--out-dir", KIN,
                         "--period", "Spring_2017:2017-01_ver56_kphighrap", "--period", "Spring_2018:2018-01_ver03_kphighrap",
                         "--period", "Fall_2018:2018-08_ver02_kphighrap"]
    assert plot[12:14] == ["--var", "chisqndf:tr: ; #chi^{2}_{#nu}; arb. unit"]
    assert "beam_vertexZ:tl: ; Z_{#lower[-0.2]{#it{prod}}} (cm); arb. unit" in plot
    assert plot[-2:] == ["--truth-var", "xim_costheta_hf:tr: ; cos#vartheta_{#it{h}}^{#Xi^{-}}; arb. unit"]


def test_steps_no_selected_study_has_are_an_error(cfg):
    with pytest.raises(gconfig.ConfigError, match=re.escape("no selected study has step(s) fit")):
        stage.run_studies(cfg, ["fit"], dry_run=True, environ=ENV, study_names=["kinematics"])


def test_period_without_tag_is_an_error(cfg):
    cfg = copy.deepcopy(cfg)
    del cfg["studies"]["kinematics"]["tags"]["2018-08"]
    with pytest.raises(gconfig.ConfigError, match=re.escape("studies.kinematics.tags.2018-08: required")):
        config.validate(cfg)


def test_truth_var_needs_a_data_histogram(cfg):
    cfg = copy.deepcopy(cfg)
    cfg["studies"]["kinematics"]["truth_vars"].append({"var": "beam_E_truth", "title": " ; E; arb. unit"})
    with pytest.raises(gconfig.ConfigError, match="'beam_E_truth' is not in vars"):
        config.validate(cfg)


def test_datamc_plot_preflight_names_the_fill_command(cfg, tmp_path, capsys):
    rc = stage.run_studies(cfg, ["plot"], runner=lambda argv, **k: None, environ=_env(tmp_path),
                           study_names=["kinematics"])
    assert rc == 1
    assert (f"{tmp_path}/o/kpkpxim/data_mc_kinematics/kinematics_kphighrap.root "
            "(gxana run studies --channel kpkpxim --study kinematics --steps fill)") in capsys.readouterr().err


def _with_threads(cfg, value=None):
    cfg = copy.deepcopy(cfg)
    for name in ("chisqndf_scan", "kinematics"):
        if value is None:
            cfg["studies"][name].pop("threads", None)
        else:
            cfg["studies"][name]["threads"] = value
    cfg["studies"]["mm2_scan"].pop("threads", None)
    return cfg


def test_threads_reaches_fill(cfg):
    cmds = stage.plan(_with_threads(cfg, 0), stage.STEPS, ["chisqndf_scan", "kinematics"], ENV)
    for c in cmds:
        if c.step == "fill":
            assert c.argv[c.argv.index("--threads") + 1] == "0" and c.argv.count("--threads") == 1
        else:
            assert "--threads" not in c.argv
    assert {c.argv[0] for c in cmds if c.step == "fill"} == {"gxana_study_cutscan", "gxana_study_datamc"}


def test_threads_absent_leaves_argv(cfg):
    cmds = stage.plan(_with_threads(cfg), stage.STEPS, ["chisqndf_scan", "kinematics"], ENV)
    assert all("--threads" not in c.argv for c in cmds)


@pytest.mark.parametrize("value", [-1, 1.5, "2", True])
@pytest.mark.parametrize("study", ["chisqndf_scan", "kinematics"])
def test_threads_invalid(cfg, study, value):
    bad = _with_threads(cfg)
    bad["studies"][study]["threads"] = value
    with pytest.raises(gconfig.ConfigError, match=rf"studies\.{study}\.threads: need an integer >= 0"):
        config.validate(bad)


def test_kpkpxim_studies_single_threaded(cfg):
    for name in ("chisqndf_scan", "mm2_scan", "kinematics"):
        fills = [c for c in stage.plan(cfg, ["fill"], [name], ENV)]
        assert fills and all(c.argv[c.argv.index("--threads") + 1] == "0" for c in fills), name
