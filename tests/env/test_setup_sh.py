import os
import shutil
import subprocess

import pytest

from gxana.paths import repo_root

ROOT = repo_root()
SHELLS = [s for s in ("bash", "zsh") if shutil.which(s)]


def sourced_env(shell, args="", extra_env=None, times=1):
    env = {"PATH": os.environ["PATH"], "HOME": os.environ.get("HOME", "/tmp"), "USER": "tester"}
    env.update(extra_env or {})
    source_cmd = f'source "{ROOT}/env/setup.sh" {args}'
    script = " && ".join([source_cmd] * times + ["env"])
    out = subprocess.run([shell, "-c", script], env=env, cwd="/", capture_output=True, text=True)
    return out.returncode, dict(line.split("=", 1) for line in out.stdout.splitlines() if "=" in line), out.stderr


@pytest.mark.parametrize("shell", SHELLS)
@pytest.mark.skipif((ROOT / "env" / "site.sh").exists(), reason="env/site.sh present; would override defaults")
def test_defaults(shell):
    rc, env, err = sourced_env(shell)
    assert rc == 0, err
    assert env["GXANA_ROOT"] == str(ROOT)
    assert env["GXANA_DATA"] == f"{ROOT}/_data"
    assert env["GXANA_OUTPUT"] == f"{ROOT}/_output"
    assert env["GXANA_EXTERNALS"] == f"{ROOT}/_externals"
    assert env["GXANA_ANALYSIS_DATA"] == f"{ROOT}/gluex_analysis_data"
    assert env["GXANA_SCRATCH"].endswith("/gxana-tester")
    assert env["PYTHONPATH"].split(":")[0] == f"{ROOT}/packages/common/python"
    assert env["PYTHONPATH"].split(":")[1] == f"{ROOT}/packages/xsection/python"
    assert env["LD_LIBRARY_PATH"].split(":")[0] == f"{ROOT}/build/lib"


@pytest.mark.parametrize("shell", SHELLS)
def test_preset_values_win(shell):
    rc, env, err = sourced_env(shell, extra_env={"GXANA_DATA": "/elsewhere"})
    assert rc == 0, err
    assert env["GXANA_DATA"] == "/elsewhere"


@pytest.mark.parametrize("shell", SHELLS)
def test_unknown_option_fails(shell):
    rc, _, err = sourced_env(shell, args="--bogus")
    assert rc != 0
    assert "unknown option --bogus" in err


@pytest.mark.parametrize("shell", SHELLS)
def test_gxana_function_defined_when_not_on_path(shell, tmp_path):
    # No real `gxana` on PATH (container case: pyyaml/pytest installed, but
    # not this package as a console script). Sourcing setup.sh should still
    # make `gxana --help` work, via the shell-function fallback.
    env = {"PATH": "/usr/bin:/bin", "HOME": str(tmp_path), "USER": "tester"}
    script = f'source "{ROOT}/env/setup.sh" && gxana --help'
    out = subprocess.run([shell, "-c", script], env=env, cwd="/", capture_output=True, text=True)
    assert out.returncode == 0, out.stderr
    assert "gxana" in out.stdout


@pytest.mark.parametrize("shell", SHELLS)
def test_idempotent_sourcing(shell):
    rc, env, err = sourced_env(shell, times=2)
    assert rc == 0, err
    assert env["LD_LIBRARY_PATH"].split(":").count(f"{ROOT}/build/lib") == 1
    assert env["PYTHONPATH"].split(":").count(f"{ROOT}/packages/common/python") == 1
