"""QFactors fork drivers (packages/qfactors): python3 shebangs, optional
termcolor, and the QFACTORS_SETTINGS hook that lets run configs live in
analyses/<channel>/ (spec §4.3)."""
import os
import shutil
import subprocess
import sys
from pathlib import Path

import pytest

ENGINE = Path(__file__).resolve().parents[2] / "packages" / "qfactors"
pytestmark = pytest.mark.skipif(not (ENGINE / "main.C").is_file(),
                                reason="packages/qfactors not checked out (git submodule update --init)")


def _run_without_termcolor(tmp_path, script, *args, env=None):
    """Run an engine script as __main__ with `import termcolor` made to fail, in a
    copy of the engine so nothing it writes lands in the real submodule."""
    engine = tmp_path / "engine"
    shutil.copytree(ENGINE, engine, symlinks=True, ignore=shutil.ignore_patterns(".git"))
    code = ("import sys; sys.modules['termcolor'] = None; "
            f"sys.argv = [{script!r}, *{list(args)!r}]; "
            f"exec(compile(open({script!r}).read(), {script!r}, 'exec'), {{'__name__': '__main__'}})")
    return subprocess.run([sys.executable, "-c", code], cwd=engine, env=env,
                          capture_output=True, text=True, timeout=60)


@pytest.mark.parametrize("name", ["run.py", "repeatProgressChecks.py"])
def test_driver_has_python3_shebang_and_is_executable(name):
    path = ENGINE / name
    assert path.read_text().splitlines()[0] == "#!/usr/bin/env python3"
    assert os.access(path, os.X_OK)


def test_gitignore_covers_run_outputs():
    lines = (ENGINE / ".gitignore").read_text().split()
    for pattern in ("logs*/", "histograms*/", "diagnosticPlots*/", "main", "*.so", "*.d"):
        assert pattern in lines


def test_repeat_progress_checks_runs_without_termcolor(tmp_path):
    proc = _run_without_termcolor(tmp_path, "repeatProgressChecks.py", "-h")
    assert proc.returncode == 0, proc.stderr
    assert "nProcess" in proc.stdout


def test_settings_file_overrides_run_py_defaults(tmp_path):
    # kDim >= nentries makes run.py print a message and exit before doing
    # anything; the upstream defaults (kDim=50, nentries=75) never do.
    settings = tmp_path / "run_settings.py"
    settings.write_text("_SET_kDim = 10\n_SET_nentries = 5\n")
    proc = _run_without_termcolor(tmp_path, "run.py", "11", env=dict(os.environ, QFACTORS_SETTINGS=str(settings)))
    assert "We cannot have kDim >= nentries" in proc.stdout, proc.stdout + proc.stderr
