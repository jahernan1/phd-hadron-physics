import sys

from gxana import doctor


def full_env(tmp_path):
    rah = tmp_path / "gluex_root_analysis"
    (rah / "scripts").mkdir(parents=True)
    (rah / "scripts" / "Load_DSelector.C").write_text("{}")
    env = {v: f"/x/{v}" for v in doctor.REQUIRED_ENV}
    env["ROOT_ANALYSIS_HOME"] = str(rah)
    return env


def all_tools(name):
    return f"/usr/bin/{name}"


def test_all_ok(tmp_path):
    lines = []
    assert doctor.main(full_env(tmp_path), all_tools, lines.append) == 0
    assert all(line.startswith("[  ok]") for line in lines)


def test_missing_env_fails(tmp_path):
    env = full_env(tmp_path)
    del env["GXANA_DATA"]
    lines = []
    assert doctor.main(env, all_tools, lines.append) == 1
    assert any(line.startswith("[fail] GXANA_DATA") for line in lines)


def test_missing_root_fails_but_cmake_only_warns(tmp_path):
    def which(name):
        return None if name in ("root", "cmake") else f"/usr/bin/{name}"

    checks = {c.name: c.status for c in doctor.run_checks(full_env(tmp_path), which)}
    assert checks["root"] == "fail"
    assert checks["cmake"] == "warn"


def test_root_analysis_home_is_warning(tmp_path):
    env = full_env(tmp_path)
    del env["ROOT_ANALYSIS_HOME"]
    checks = {c.name: c.status for c in doctor.run_checks(env, all_tools)}
    assert checks["ROOT_ANALYSIS_HOME"] == "warn"


def test_doctor_runs_without_pyyaml_installed(tmp_path, monkeypatch):
    # pyyaml must only be imported lazily, so `gxana doctor` itself never
    # raises ImportError even when pyyaml is not importable; it should just
    # report that one check as "fail".
    monkeypatch.setitem(sys.modules, "yaml", None)
    lines = []
    rc = doctor.main(full_env(tmp_path), all_tools, lines.append)
    assert rc == 1
    assert any(line.startswith("[fail] pyyaml") for line in lines)


def test_gxenv_missing_is_warn_not_fail(tmp_path):
    def which(name):
        return None if name == "gxenv" else f"/usr/bin/{name}"

    env = full_env(tmp_path)
    checks = {c.name: c.status for c in doctor.run_checks(env, which)}
    assert checks["gxenv"] == "warn"
    # A missing gxenv must never fail doctor overall.
    assert doctor.main(env, which, lambda line: None) == 0
