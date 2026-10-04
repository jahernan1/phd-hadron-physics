import subprocess
from pathlib import Path

import pytest

from gxana.config import load_channel
from gxana.stages import select


@pytest.fixture(scope="module")
def cfg():
    return load_channel("kpkpxim")


@pytest.fixture
def env(tmp_path):
    e = {k: str(tmp_path / k.lower()) for k in ("GXANA_DATA", "GXANA_OUTPUT", "GXANA_SCRATCH")}
    e["ROOT_ANALYSIS_HOME"] = "/opt/gluex_root_analysis"
    return e


class FakeRoot:
    """Stands in for `rootls` and `root`; `root` drops `produce` files in its cwd."""

    def __init__(self, produce=(), keys="kpkpxim__B4_M23_Tree\n", rc=0):
        self.produce, self.keys, self.rc, self.calls = produce, keys, rc, []

    def __call__(self, cmd, **kw):
        self.calls.append((cmd, kw))
        if cmd[0] == "rootls":
            return subprocess.CompletedProcess(cmd, 0, stdout=self.keys, stderr="")
        for name in self.produce:
            (Path(kw["cwd"]) / name).write_text("new")
        return subprocess.CompletedProcess(cmd, self.rc)


def make_job(cfg, env, tmp_path, **kw):
    selector = tmp_path / "DSelector_kpkpxim.C"
    selector.write_text("// selector")
    job = select.plan_select(cfg, kw.pop("period", "2018-08"), kw.pop("sample", "data"),
                             selector=str(selector), environ=env, **kw)
    job.tree_dir.mkdir(parents=True)
    for i in (1, 2):
        (job.tree_dir / f"tree_{i}.root").write_text("t")
    return job


def test_plan_names_match_legacy(cfg, env):
    job = select.plan_select(cfg, "2017-01", "data", environ=env)
    assert job.save_name == "kpkpxim__M23_2017-01_ana56"
    assert job.tree_dir == Path(env["GXANA_DATA"]) / "Trees/tree_kpkpxim__M23_2017-01_ana56/trees"
    assert job.selector.name == "DSelector_kpkpxim.C"
    assert job.selector.parent.parts[-3:] == ("analyses", "kpkpxim", "selectors")
    assert job.cores == 16
    assert job.run_dir == Path(env["GXANA_SCRATCH"]) / "run" / job.save_name
    assert job.hist_dir == Path(env["GXANA_OUTPUT"]) / "kpkpxim" / "selector_hists"
    assert job.flat_dir == Path(env["GXANA_DATA"]) / "Trees" / "flatTree" / "rawTrees"
    assert job.thrown_dir == Path(env["GXANA_DATA"]) / "flatTrees"


def test_plan_thrown_with_tag(cfg, env):
    job = select.plan_select(cfg, "2018-01", "gen_amp_V2_ac_YstarRest", thrown=True, tag="oneRfBunch", environ=env)
    assert job.save_name == "kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_ac_YstarRest_oneRfBunch"
    assert job.selector.name == "DSelector_thrown_kpkpxim.C"


def test_plan_resolves_relative_paths(cfg, monkeypatch, tmp_path):
    # A relative --selector and relative GXANA_DATA must not depend on the
    # cwd ROOT is later launched from (job.run_dir); everything in the job,
    # and in the generated ROOT script, must be absolute.
    monkeypatch.chdir(tmp_path)
    (tmp_path / "sel").mkdir()
    (tmp_path / "sel" / "DSelector_kpkpxim.C").write_text("// selector")
    env = {
        "GXANA_DATA": "reldata",
        "GXANA_OUTPUT": str(tmp_path / "out"),
        "GXANA_SCRATCH": str(tmp_path / "scratch"),
    }
    job = select.plan_select(cfg, "2018-08", "data", selector="sel/DSelector_kpkpxim.C", environ=env)
    assert job.selector.is_absolute()
    assert job.selector == (tmp_path / "sel" / "DSelector_kpkpxim.C").resolve()
    assert job.tree_dir.is_absolute()
    assert job.tree_dir == (tmp_path / "reldata" / "Trees"
                             / "tree_kpkpxim__B4_M23_2018-08_ana02" / "trees").resolve()
    assert job.run_dir.is_absolute()
    assert job.sandbox.is_absolute()
    assert job.hist_dir.is_absolute()
    assert job.flat_dir.is_absolute()
    script = select.root_script(job, "kpkpxim__B4_M23_Tree", "/rah")
    assert f'ch->Add("{job.tree_dir}/*");' in script
    assert f'"{job.selector}++"' in script


def test_root_script(cfg, env):
    job = select.plan_select(cfg, "2018-08", "data", cores=8, environ=env)
    script = select.root_script(job, "kpkpxim__B4_M23_Tree", "/rah")
    assert f'gEnv->SetValue("ProofLite.Sandbox", "{env["GXANA_SCRATCH"]}/proof");' in script
    assert ".x /rah/scripts/Load_DSelector.C" in script
    assert 'TChain *ch = new TChain("kpkpxim__B4_M23_Tree");' in script
    assert f'ch->Add("{job.tree_dir}/*");' in script
    assert f'DPROOFLiteManager::Process_Chain(ch, "{job.selector}++", 8);' in script


def test_run_moves_data_outputs(cfg, env, tmp_path):
    job = make_job(cfg, env, tmp_path)
    fake = FakeRoot(produce=("kpkpxim.root", "flatTree_kpkpxim.root"))
    assert select.run_select(job, environ=env, runner=fake, log=lambda *_: None) == 0
    assert (job.hist_dir / "kpkpxim__B4_M23_2018-08_ana02.root").is_file()
    assert (job.flat_dir / "flatTree_kpkpxim__B4_M23_2018-08_ana02.root").is_file()
    rootls_cmd, root_call = fake.calls[0][0], fake.calls[1]
    assert rootls_cmd == ["rootls", str(job.tree_dir / "tree_1.root")]
    assert root_call[0] == ["root", "-l", "-b"]
    assert root_call[1]["cwd"] == job.run_dir
    assert "Process_Chain" in root_call[1]["input"]


def test_run_moves_thrown_outputs(cfg, env, tmp_path):
    job = make_job(cfg, env, tmp_path)
    fake = FakeRoot(produce=("thrown_kpkpxim.root", "flatTree_thrown_kpkpxim.root"))
    assert select.run_select(job, environ=env, runner=fake, log=lambda *_: None) == 0
    assert (job.hist_dir / "thrown_kpkpxim__B4_M23_2018-08_ana02.root").is_file()
    assert (job.thrown_dir / "flatTree_thrown_kpkpxim__B4_M23_2018-08_ana02.root").is_file()
    assert not (job.flat_dir / "flatTree_thrown_kpkpxim__B4_M23_2018-08_ana02.root").exists()


def test_run_removes_stale_outputs(cfg, env, tmp_path):
    job = make_job(cfg, env, tmp_path)
    job.run_dir.mkdir(parents=True)
    (job.run_dir / "flatTree_kpkpxim.root").write_text("stale")
    fake = FakeRoot(produce=("kpkpxim.root",))
    assert select.run_select(job, environ=env, runner=fake, log=lambda *_: None) == 0
    assert not (job.flat_dir / "flatTree_kpkpxim__B4_M23_2018-08_ana02.root").exists()


def test_run_nothing_produced_returns_1(cfg, env, tmp_path):
    job = make_job(cfg, env, tmp_path)
    assert select.run_select(job, environ=env, runner=FakeRoot(), log=lambda *_: None) == 1


def test_root_failure_propagates(cfg, env, tmp_path):
    job = make_job(cfg, env, tmp_path)
    fake = FakeRoot(produce=("kpkpxim.root",), rc=3)
    assert select.run_select(job, environ=env, runner=fake, log=lambda *_: None) == 3
    assert not job.hist_dir.exists()


def test_multiple_tree_keys_rejected(cfg, env, tmp_path):
    job = make_job(cfg, env, tmp_path)
    with pytest.raises(select.SelectError, match="exactly one key"):
        select.run_select(job, environ=env, runner=FakeRoot(keys="a b\n"), log=lambda *_: None)


def test_requires_root_analysis_home(cfg, env, tmp_path):
    job = make_job(cfg, env, tmp_path)
    del env["ROOT_ANALYSIS_HOME"]
    with pytest.raises(select.SelectError, match="ROOT_ANALYSIS_HOME"):
        select.run_select(job, environ=env, runner=FakeRoot(), log=lambda *_: None)


def test_missing_executable_wrapped_as_select_error(cfg, env, tmp_path):
    job = make_job(cfg, env, tmp_path)

    def missing_root(cmd, **kw):
        if cmd[0] == "rootls":
            return subprocess.CompletedProcess(cmd, 0, stdout="kpkpxim__B4_M23_Tree\n", stderr="")
        raise FileNotFoundError("root: command not found")

    with pytest.raises(select.SelectError, match="root"):
        select.run_select(job, environ=env, runner=missing_root, log=lambda *_: None)


def test_rootls_failure_wrapped_as_select_error(cfg, env, tmp_path):
    job = make_job(cfg, env, tmp_path)

    def failing_rootls(cmd, **kw):
        raise subprocess.CalledProcessError(1, cmd, output="", stderr="rootls: not a root file")

    with pytest.raises(select.SelectError, match="rootls"):
        select.run_select(job, environ=env, runner=failing_rootls, log=lambda *_: None)


def test_run_never_writes_through_a_linked_destination(cfg, env, tmp_path, monkeypatch):
    import errno
    import os

    job = make_job(cfg, env, tmp_path)
    preserved = tmp_path / "preserved.root"
    preserved.write_bytes(b"preserved")
    dst = job.thrown_dir / "flatTree_thrown_kpkpxim__B4_M23_2018-08_ana02.root"
    dst.parent.mkdir(parents=True)
    dst.symlink_to(preserved)

    def exdev(*_args, **_kwargs):
        raise OSError(errno.EXDEV, "cross-device link")

    monkeypatch.setattr(os, "rename", exdev)  # shutil.move falls back to copy + unlink
    fake = FakeRoot(produce=("thrown_kpkpxim.root", "flatTree_thrown_kpkpxim.root"))
    assert select.run_select(job, environ=env, runner=fake, log=lambda *_: None) == 0
    assert preserved.read_bytes() == b"preserved"
    assert dst.is_file() and not dst.is_symlink()
    assert not list(dst.parent.glob("*.part"))
