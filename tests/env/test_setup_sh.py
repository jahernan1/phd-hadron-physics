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


def _stub_boot(tmp_path):
    boot = tmp_path / "boot.sh"
    boot.write_text(f'gxenv() {{ echo "$1" > "{tmp_path}/gxenv_arg"; }}\n')
    return boot


@pytest.mark.parametrize("shell", SHELLS)
def test_sim_renders_and_runs_gxenv(shell, tmp_path):
    ext = tmp_path / "ext"
    rc, env, err = sourced_env(shell, "--sim=recon-2018_08-ver02_31",
                               {"GXANA_GLUEX_BOOT": str(_stub_boot(tmp_path)), "GXANA_EXTERNALS": str(ext)})
    assert rc == 0, err
    rendered = ext / "version_sets" / "recon-2018_08-ver02_31.xml"
    assert (tmp_path / "gxenv_arg").read_text().strip() == str(rendered)
    text = rendered.read_text()
    assert f'home="{ext}/halld_sim-recon-2018_08-ver02_31"' in text
    assert f'home="{ext}/gluex_MCwrapper"' in text
    assert "${" not in text
    assert env["GXANA_SIM_VERSION_SET"] == "recon-2018_08-ver02_31"


@pytest.mark.parametrize("shell", SHELLS)
def test_sim_unknown_set_fails(shell, tmp_path):
    rc, _, err = sourced_env(shell, "--sim=recon-bogus", {"GXANA_GLUEX_BOOT": str(_stub_boot(tmp_path))})
    assert rc != 0
    assert "unknown sim version set recon-bogus" in err


@pytest.mark.parametrize("shell", SHELLS)
def test_sim_and_gluex_are_exclusive(shell, tmp_path):
    rc, _, err = sourced_env(shell, "--gluex --sim=recon-2018_08-ver02_31",
                             {"GXANA_GLUEX_BOOT": str(_stub_boot(tmp_path))})
    assert rc != 0
    assert "--gluex and --sim are exclusive" in err


@pytest.mark.parametrize("shell", SHELLS)
def test_sim_missing_boot_fails(shell, tmp_path):
    rc, _, err = sourced_env(shell, "--sim=recon-2018_08-ver02_31",
                             {"GXANA_GLUEX_BOOT": str(tmp_path / "nope.sh"), "GXANA_EXTERNALS": str(tmp_path / "e")})
    assert rc != 0
    assert "not found" in err
