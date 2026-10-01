"""gxana run qfactors: plan, render, stage and run (pure logic; run.py and g++ are faked)."""
import os
import shutil
import subprocess
from pathlib import Path

import pytest

from gxana import config
from gxana.cli import main
from gxana.paths import repo_root
from gxana.stages import qfactors

CFG = config.load_channel("kpkpxim")
ENGINE = repo_root() / "packages" / "qfactors"
needs_engine = pytest.mark.skipif(not (ENGINE / "main.C").is_file(), reason="packages/qfactors not checked out")
FIXTURE = Path(__file__).with_name("data") / "configSettings_thesis_2018-08.h"


@pytest.fixture
def env(tmp_path):
    return {"GXANA_ROOT": str(repo_root()), **{v: str(tmp_path / v.lower()) for v in (
        "GXANA_DATA", "GXANA_OUTPUT", "GXANA_SCRATCH", "GXANA_EXTERNALS", "GXANA_ANALYSIS_DATA")}}


def _touch_input(job):
    job.input_file.parent.mkdir(parents=True, exist_ok=True)
    job.input_file.write_text("")


@needs_engine
@pytest.mark.parametrize("period,stem", [
    ("2017-01", "kpkpxim__M23_2017-01_ana56"),
    ("2018-01", "kpkpxim__B4_M23_2018-01_ana03"),
    ("2018-08", "kpkpxim__B4_M23_2018-08_ana02"),
])
def test_plan_thesis_periods_and_result_is_xsection_input(period, stem, env):
    job = qfactors.plan_qfactors(CFG, period, environ=env)
    tag = f"{stem}_nominal_kphighrap"
    assert (job.stem, job.file_tag, job.combo_tag) == (stem, tag, tag + "_1111111")
    assert job.input_file == Path(env["GXANA_DATA"]) / "flatTrees" / f"flatTree_{tag}.root"
    assert job.work_dir == Path(env["GXANA_SCRATCH"]) / "qfactors" / tag
    assert job.pdf_config == ENGINE / CFG["qfactors"]["model"]
    data_input = config.expand_env(CFG["xsection"]["inputs"]["data"], env).format(stem=stem)
    assert str(job.result) == data_input


@needs_engine
def test_render_matches_thesis_configsettings(env):
    job = qfactors.plan_qfactors(CFG, "2018-08", environ=env)
    rendered = qfactors.render_config_settings((ENGINE / "configSettings.h").read_text(), job)
    expected = (FIXTURE.read_text().replace("@CWD@", str(job.work_dir.resolve()))
                .replace("@INPUT@", str(job.input_file)))
    assert rendered == expected


@needs_engine
def test_render_keeps_run_py_substring_semantics(env):
    job = qfactors.plan_qfactors(CFG, "2018-08", environ=env)
    template = ("int kDim=50;\nconst int ckDim=50; // same as kDim but just of const int type\n"
                "Long64_t nentries=75;\nbool override_nentries=1;\nconst int extraVarDim=0;\n")
    assert qfactors.render_config_settings(template, job) == (
        "int kDim=200;\nconst int ckDim=200; // same as kDim but just of const int type\n"
        "Long64_t nentries=-1;\nbool override_nentries=0;\nconst int extraVarDim=1;\n")


@needs_engine
def test_empty_optional_var_string_gives_dim_zero(env):
    job = qfactors.plan_qfactors(CFG, "2018-08", overrides={"extraVars": ""}, environ=env)
    assert qfactors.render_config_settings("const int extraVarDim=1;\n", job) == "const int extraVarDim=0;\n"


@needs_engine
def test_settings_py_round_trips(env):
    job = qfactors.plan_qfactors(CFG, "2017-01", overrides={"nProcess": 4}, environ=env)
    scope = {}
    exec(qfactors.settings_py(job), scope)
    assert scope["rootFileLocs"] == [(str(job.input_file), "flatTree_kpkpxim", job.file_tag)]
    assert scope["_SET_nProcess"] == 4
    assert {k[5:]: v for k, v in scope.items() if k.startswith("_SET_")} == job.settings


@needs_engine
def test_unknown_model_lists_known(env):
    with pytest.raises(config.ConfigError, match=r"unknown Q-factor model 'configPDFs_Voigt.h'; known: .*'configPDFs.h'"):
        qfactors.plan_qfactors(CFG, "2017-01", model="configPDFs_Voigt.h", environ=env)


@pytest.mark.parametrize("overrides,match", [
    ({"kDimm": 3}, "unknown qfactors setting"),
    ({"runBatch": 1}, "runBatch"),
    ({"runAllPhaseCombos": 1}, "runAllPhaseCombos"),
    ({"runTag": "_x"}, "runTag"),
])
def test_unsupported_settings_rejected(overrides, match, env):
    with pytest.raises(config.ConfigError, match=match):
        qfactors.plan_qfactors(CFG, "2017-01", overrides=overrides, environ=env)


def test_missing_setting_rejected(env):
    cfg = {**CFG, "qfactors": {**CFG["qfactors"], "settings": {k: v for k, v in CFG["qfactors"]["settings"].items()
                                                               if k != "kDim"}}}
    with pytest.raises(config.ConfigError, match="missing qfactors setting.*kDim"):
        qfactors.plan_qfactors(cfg, "2017-01", environ=env)


@needs_engine
def test_stage_builds_work_dir(env):
    job = qfactors.plan_qfactors(CFG, "2017-01", environ=env)
    qfactors.stage(job)
    w = job.work_dir
    assert (w / "main.C").read_bytes() == (ENGINE / "main.C").read_bytes()
    assert (w / "configPDFs.h").read_bytes() == (ENGINE / "configPDFs.h").read_bytes()
    assert (w / "logs").resolve() == job.output_dir.resolve()
    assert (w / "histograms").resolve() == (job.plots_dir / "histograms").resolve()
    assert (w / "diagnosticPlots").resolve() == (job.plots_dir / "diagnosticPlots").resolve()
    assert (w / "makePlotsVars.txt").read_text().splitlines() == CFG["qfactors"]["diagnostic_vars"]
    assert f'fileTag="{job.combo_tag}";' in (w / "configSettings.h").read_text()
    assert (w / "run_settings.py").read_text() == qfactors.settings_py(job)
    assert (job.output_dir / job.combo_tag).is_dir()
    assert not (w / ".git").exists()


@needs_engine
@pytest.mark.parametrize("model", sorted(p.name for p in ENGINE.glob("configPDFs*.h")))
def test_stage_uses_named_variant(model, env):
    job = qfactors.plan_qfactors(CFG, "2017-01", model=model, environ=env)
    assert (job.model, job.pdf_config) == (model, ENGINE / model)
    qfactors.stage(job)
    assert (job.work_dir / "configPDFs.h").read_bytes() == (ENGINE / model).read_bytes()


@needs_engine
def test_channel_model_path_from_the_yaml_is_relative_to_the_repository_root(env, tmp_path):
    rel = "analyses/synthch/config/qfactors_models/configPDFs_X.h"
    (tmp_path / rel).parent.mkdir(parents=True)
    shutil.copyfile(ENGINE / "configPDFs_Johnson.h", tmp_path / rel)
    cfg = {**CFG, "qfactors": {**CFG["qfactors"], "engine_dir": str(ENGINE), "model": rel}}
    job = qfactors.plan_qfactors(cfg, "2017-01", environ={**env, "GXANA_ROOT": str(tmp_path)})
    assert (job.model, job.pdf_config, job.engine_dir) == (rel, tmp_path / rel, ENGINE)
    qfactors.stage(job)
    assert (job.work_dir / "configPDFs.h").read_bytes() == (ENGINE / "configPDFs_Johnson.h").read_bytes()
    assert (job.work_dir / "configPDFs_Johnson.h").read_bytes() == (ENGINE / "configPDFs_Johnson.h").read_bytes()


@needs_engine
@pytest.mark.parametrize("model", [
    "analyses/kpkpxim/config/qfactors_models/configPDFs_none.h",   # path that does not exist
    "analyses/kpkpxim/config",                                      # a directory
    "README.md",                                                    # a bare name is only an engine model
])
def test_unknown_channel_model_path_lists_both_forms(model, env):
    with pytest.raises(config.ConfigError) as err:
        qfactors.plan_qfactors(CFG, "2017-01", model=model, environ=env)
    msg = str(err.value)
    assert msg.startswith(f"unknown Q-factor model {model!r}; known: [") and "'configPDFs.h'" in msg
    assert "or the path, relative to the repository root, of an existing channel model file" in msg


@needs_engine
def test_stage_relinks_stale_symlink_and_refuses_real_dir(env, tmp_path):
    job = qfactors.plan_qfactors(CFG, "2017-01", environ=env)
    job.work_dir.mkdir(parents=True)
    (job.work_dir / "logs").symlink_to(tmp_path, target_is_directory=True)
    qfactors.stage(job)
    assert (job.work_dir / "logs").resolve() == job.output_dir.resolve()
    (job.work_dir / "histograms").unlink()
    (job.work_dir / "histograms").mkdir()
    with pytest.raises(qfactors.QFactorsError, match="histograms is a real directory"):
        qfactors.stage(job)


def test_plan_requires_checked_out_engine(env, tmp_path):
    (tmp_path / "empty").mkdir()
    cfg = {**CFG, "qfactors": {**CFG["qfactors"], "engine_dir": str(tmp_path / "empty")}}
    with pytest.raises(config.ConfigError, match="not a checked-out QFactors engine; "
                                                 "run `git submodule update --init packages/qfactors`"):
        qfactors.plan_qfactors(cfg, "2017-01", environ=env)


@needs_engine
def test_stage_requires_checked_out_engine(env, tmp_path):
    job = qfactors.plan_qfactors(CFG, "2017-01", environ=env)._replace(engine_dir=tmp_path / "empty")
    (tmp_path / "empty").mkdir()
    with pytest.raises(qfactors.QFactorsError, match="git submodule update --init packages/qfactors"):
        qfactors.stage(job)


def test_run_py_arg():
    assert qfactors.run_py_arg(["fit", "plots"]) == "11"
    assert qfactors.run_py_arg(["fit"]) == "10"
    assert qfactors.run_py_arg(["plots"]) == "01"
    assert qfactors.run_py_arg(["prepare"]) is None
    with pytest.raises(qfactors.QFactorsError, match="unknown step"):
        qfactors.run_py_arg(["fits"])


@needs_engine
def test_run_refuses_missing_input_before_launch(env):
    job = qfactors.plan_qfactors(CFG, "2017-01", environ=env)
    calls = []
    with pytest.raises(qfactors.QFactorsError, match="input flat tree not found"):
        qfactors.run_qfactors(job, ["fit", "plots"], environ=env, runner=lambda *a, **k: calls.append(a),
                              which=lambda name: "/usr/bin/" + name, log=lambda s: None)
    assert calls == []


@needs_engine
def test_run_invokes_run_py_with_settings_env(env):
    job = qfactors.plan_qfactors(CFG, "2017-01", environ=env)
    _touch_input(job)
    calls = []

    def runner(argv, **kw):
        calls.append((argv, kw))
        job.result.write_text("")
        return subprocess.CompletedProcess(argv, 0, "", "")

    rc = qfactors.run_qfactors(job, ["fit", "plots"], environ=env, runner=runner,
                               which=lambda name: "/usr/bin/" + name, log=lambda s: None)
    assert rc == 0
    (argv, kw), = calls
    assert argv[1:] == ["run.py", "11"] and kw["cwd"] == job.work_dir
    assert kw["env"]["QFACTORS_SETTINGS"] == str(job.work_dir / "run_settings.py")


def _run_with_fake_run_py(job, steps, env, writes_result):
    """run_qfactors with a run.py that exits 0 and writes job.result only if asked; returns (rc, log lines)."""
    lines = []

    def runner(argv, **kw):
        if writes_result:
            job.result.write_text("")
        return subprocess.CompletedProcess(argv, 0, "", "")

    rc = qfactors.run_qfactors(job, steps, environ=env, runner=runner,
                               which=lambda name: "/usr/bin/" + name, log=lines.append)
    return rc, lines


@needs_engine
def test_run_fails_when_run_py_exits_0_without_result(env):
    job = qfactors.plan_qfactors(CFG, "2017-01", environ=env)
    _touch_input(job)
    rc, lines = _run_with_fake_run_py(job, ["fit", "plots"], env, writes_result=False)
    assert rc == 1
    assert not any(line.startswith("Result:") for line in lines)
    error, = [line for line in lines if line.startswith("ERROR:")]
    assert str(job.result) in error and f"{job.output_dir / job.combo_tag}/err*.txt" in error


@needs_engine
def test_run_logs_result_when_written(env):
    job = qfactors.plan_qfactors(CFG, "2017-01", environ=env)
    _touch_input(job)
    rc, lines = _run_with_fake_run_py(job, ["fit", "plots"], env, writes_result=True)
    assert rc == 0
    assert f"Result: {job.result}" in lines


@needs_engine
def test_run_fit_only_logs_no_result(env):
    job = qfactors.plan_qfactors(CFG, "2017-01", environ=env)
    _touch_input(job)
    rc, lines = _run_with_fake_run_py(job, ["fit"], env, writes_result=False)
    assert rc == 0
    assert not any(line.startswith(("Result:", "ERROR:")) for line in lines)


@needs_engine
def test_prepare_compiles_with_root_flags(env):
    job = qfactors.plan_qfactors(CFG, "2017-01", environ=env)
    _touch_input(job)
    calls = []

    def runner(argv, **kw):
        calls.append(argv)
        out = "-I/root/include -L/root/lib -lCore\n" if argv[0] == "root-config" else ""
        return subprocess.CompletedProcess(argv, 0, out, "")

    assert qfactors.run_qfactors(job, ["prepare"], environ=env, runner=runner,
                                 which=lambda name: "/usr/bin/" + name, log=lambda s: None) == 0
    assert calls[-1] == ["g++", "-o", "main", "main.C", "-I/root/include", "-L/root/lib", "-lCore",
                         "-lRooStats", "-lRooFitCore", "-lRooFit"]


@needs_engine
def test_cli_dry_run(env, capsys, monkeypatch):
    for key, value in env.items():
        monkeypatch.setenv(key, value)
    assert main(["run", "qfactors", "--period", "2018-01", "--dry-run"]) == 0
    out = capsys.readouterr().out
    assert "kpkpxim__B4_M23_2018-01_ana03_nominal_kphighrap_1111111" in out
    assert "QFACTORS_SETTINGS=" in out and "run.py 11" in out


@needs_engine
def test_cli_dry_run_prints_a_channel_model_path(env, capsys, monkeypatch):
    for key, value in env.items():
        monkeypatch.setenv(key, value)
    assert main(["run", "qfactors", "--period", "2018-01", "--model", "packages/qfactors/configPDFs_Johnson.h",
                 "--dry-run"]) == 0
    out = capsys.readouterr().out
    assert f"model:     packages/qfactors/configPDFs_Johnson.h ({ENGINE / 'configPDFs_Johnson.h'})" in out


@needs_engine
def test_cli_unknown_step_is_an_error(env, capsys, monkeypatch):
    for key, value in env.items():
        monkeypatch.setenv(key, value)
    assert main(["run", "qfactors", "--period", "2018-01", "--steps", "fits", "--dry-run"]) == 2
    assert "unknown step" in capsys.readouterr().err
