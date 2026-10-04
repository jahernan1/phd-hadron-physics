"""gxana run mc: plan, render and submit (pure logic; gluex_MC.py is faked)."""
import re
import subprocess
from pathlib import Path

import pytest

from gxana import config
from gxana.cli import main
from gxana.paths import MissingEnvError, repo_root
from gxana.stages import mc

CFG = config.load_channel("kpkpxim")
LEGACY = re.compile(r"/d/grid1[37]/|/work/halld/home/|/w/halld-scshelf")


@pytest.fixture
def env(tmp_path):
    return {"GXANA_ROOT": str(repo_root()), **{v: str(tmp_path / v.lower()) for v in (
        "GXANA_DATA", "GXANA_OUTPUT", "GXANA_SCRATCH", "GXANA_EXTERNALS", "GXANA_ANALYSIS_DATA")}}


def test_override_keys_keeps_comments_and_skips_commented_lines():
    text = "#GENERATOR_CONFIG=/old\nGENERATOR=gen_amp_V2 #or file\nGENERATOR_CONFIG=/x/y.cfg\nBKG=Random:a #[None, ...]\nACCOUNT = halld   # url\nOS=el9\n"
    out = mc.override_keys(text, {"GENERATOR_CONFIG": "/new.cfg", "BKG": "None", "ACCOUNT": "other", "OS": "el8"})
    assert out == ("#GENERATOR_CONFIG=/old\nGENERATOR=gen_amp_V2 #or file\nGENERATOR_CONFIG=/new.cfg\n"
                   "BKG=None #[None, ...]\nACCOUNT = other   # url\nOS=el8\n")


def test_override_keys_requires_exactly_one_line():
    with pytest.raises(mc.McError, match="expected one NCORES= line"):
        mc.override_keys("A=1\n", {"NCORES": "2"})
    with pytest.raises(mc.McError, match="found 2"):
        mc.override_keys("A=1\nA=2\n", {"A": "3"})


@pytest.mark.parametrize("period,runs,events,vs", [
    ("2017-01", "30274-31057", "3735000", "recon-2019_11-ver01_13"),
    ("2018-01", "40856-42559", "11190000", "recon-2018_01-ver02_32"),
    ("2018-08", "50685-51768", "7055000", "recon-2018_08-ver02_31"),
])
def test_plan_thesis_periods(env, period, runs, events, vs):
    job = mc.plan_mc(CFG, period, "gen_amp_V2_ac_YstarRest", env)
    stem = config.tree_stem(CFG, period, "gen_amp_V2_ac_YstarRest")
    assert job.run_dir == Path(env["GXANA_OUTPUT"]) / "kpkpxim" / "mc" / f"tree_{stem}"
    assert job.argv == ["gluex_MC.py", str(job.conf), runs, events, "batch=2"]
    assert job.version_set == vs
    assert job.overrides["ENVIRONMENT_FILE"] == f"{env['GXANA_EXTERNALS']}/version_sets/{vs}.xml"
    assert job.overrides["DATA_OUTPUT_BASE_DIR"] == str(job.run_dir)
    assert job.overrides["GENERATOR_CONFIG"] == str(job.run_dir / "kpkpxim_2dhist_ac_YstarRest.cfg")
    assert job.overrides["WORKFLOW_NAME"] == f"kpkpxim_{period}_gen_amp_V2_ac_YstarRest"


def test_links_point_where_select_reads(env):
    job = mc.plan_mc(CFG, "2018-08", "gen_amp_V2_ac_YstarRest", env)
    for kind, (link, target) in zip(("trees", "thrown"), job.links):
        assert str(link) == f"{env['GXANA_DATA']}/" + config.tree_dir(CFG, "2018-08", "gen_amp_V2_ac_YstarRest", kind).rstrip("/")
        assert target == job.run_dir / "root" / kind


def test_nobkg_only_2018_08(env):
    job = mc.plan_mc(CFG, "2018-08", "gen_amp_V2_nobkg", env)
    assert job.overrides["BKG"] == "None"
    with pytest.raises(config.ConfigError, match="produced only for"):
        mc.plan_mc(CFG, "2017-01", "gen_amp_V2_nobkg", env)


def test_unknown_period_and_sample(env):
    with pytest.raises(config.ConfigError, match="unknown period"):
        mc.plan_mc(CFG, "2019-11", "gen_amp_V2_ac_YstarRest", env)
    with pytest.raises(config.ConfigError, match="no MC production"):
        mc.plan_mc(CFG, "2018-08", "data", env)


@pytest.mark.parametrize("sample", ["gen_amp_V2_ac_YstarRest", "gen_amp_V2_noac_YstarRest"])
@pytest.mark.parametrize("period", ["2017-01", "2018-01", "2018-08"])
def test_render_is_fully_expanded(env, period, sample):
    job = mc.plan_mc(CFG, period, sample, env)
    files = mc.render(job, env)
    assert set(files) == {job.generator_config, job.conf}
    for text in files.values():
        assert "${" not in text and not LEGACY.search(text)
    conf = files[job.conf]
    for key, value in job.overrides.items():
        assert len(re.findall(rf"^\s*{key}\s*=\s*{re.escape(value)}(\s|$)", conf, re.M)) == 1, key
    assert f"{env['GXANA_ANALYSIS_DATA']}/kpkpxim/simulation/sampling/" in files[job.generator_config]


def test_render_unset_var_names_it(env):
    job = mc.plan_mc(CFG, "2018-08", "gen_amp_V2_ac_YstarRest", env)
    del env["GXANA_OUTPUT"]  # planned already; the templates still reference it
    with pytest.raises(MissingEnvError, match="GXANA_OUTPUT"):
        mc.render(job, env)


def test_render_missing_template_raises_mcerror(env):
    job = mc.plan_mc(CFG, "2018-08", "gen_amp_V2_ac_YstarRest", env)
    job = job._replace(cfg_template=job.cfg_template.parent / "does-not-exist.cfg")
    with pytest.raises(mc.McError, match="does-not-exist.cfg"):
        mc.render(job, env)


def _ready(env, job):
    job.version_set_xml.parent.mkdir(parents=True, exist_ok=True)
    job.version_set_xml.write_text("<gversions/>")
    return dict(env, GXANA_SIM_VERSION_SET=job.version_set)


def test_run_refuses_wrong_sim_env(env):
    job = mc.plan_mc(CFG, "2018-08", "gen_amp_V2_ac_YstarRest", env)
    ready = _ready(env, job)
    ready["GXANA_SIM_VERSION_SET"] = "recon-2019_11-ver01_13"
    with pytest.raises(mc.McError, match=r"source env/setup.sh --sim=recon-2018_08-ver02_31"):
        mc.run_mc(job, ready, which=lambda _: "/x/gluex_MC.py")
    with pytest.raises(mc.McError, match="active: none"):
        mc.run_mc(job, env, which=lambda _: "/x/gluex_MC.py")


def test_run_needs_gluex_mc_py(env):
    job = mc.plan_mc(CFG, "2018-08", "gen_amp_V2_ac_YstarRest", env)
    with pytest.raises(mc.McError, match="gluex_MC.py not on PATH"):
        mc.run_mc(job, _ready(env, job), which=lambda _: None)


def test_run_writes_inputs_and_submits(env):
    job = mc.plan_mc(CFG, "2018-08", "gen_amp_V2_ac_YstarRest", env)
    calls, logs = [], []

    def runner(argv, cwd):
        calls.append((argv, cwd))
        return subprocess.CompletedProcess(argv, 0)

    assert mc.run_mc(job, _ready(env, job), runner=runner, which=lambda _: "/x/gluex_MC.py", log=logs.append) == 0
    assert calls == [(job.argv, job.run_dir)]
    assert job.conf.is_file() and job.generator_config.is_file()
    assert any(f"ln -sfn {job.run_dir}/root/trees" in line for line in logs)


def test_link_commands_use_ln_sfn(env):
    job = mc.plan_mc(CFG, "2018-08", "gen_amp_V2_ac_YstarRest", env)
    lines = mc.link_commands(job)
    assert lines
    for line in lines:
        assert "ln -sfn" in line
        assert "ln -s " not in line.replace("ln -sfn ", "")


def test_link_commands_quotes_paths_with_spaces():
    job = mc.McJob(period="p", sample="s", stem="stem", run_dir=Path("/tmp/x"),
                   cfg_template=Path("/t/c"), conf_template=Path("/t/d"),
                   generator_config=Path("/g"), conf=Path("/c"), version_set="v",
                   version_set_xml=Path("/v"), overrides={}, argv=[],
                   links=[(Path("/a b/link"), Path("/c d/target"))])
    lines = mc.link_commands(job)
    assert lines == ["mkdir -p '/a b' && ln -sfn '/c d/target' '/a b/link'"]


def test_cli_dry_run(env, monkeypatch, capsys):
    for k, v in env.items():
        monkeypatch.setenv(k, v)
    assert main(["run", "mc", "--channel", "kpkpxim", "--period", "2018-08",
                 "--sample", "gen_amp_V2_ac_YstarRest", "--dry-run"]) == 0
    out = capsys.readouterr().out
    assert "gluex_MC.py" in out and "50685-51768 7055000 batch=2" in out
    assert "source env/setup.sh --sim=recon-2018_08-ver02_31" in out
