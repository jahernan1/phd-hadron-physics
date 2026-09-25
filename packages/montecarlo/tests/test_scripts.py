"""build_halld_sim.sh argument handling and dry-run plan."""
import os
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
SCRIPT = ROOT / "packages/montecarlo/scripts/build_halld_sim.sh"


def run(*args, env=None):
    return subprocess.run(["bash", str(SCRIPT), *args], capture_output=True, text=True,
                          env=dict(os.environ, **(env or {})))


def test_syntax():
    assert subprocess.run(["bash", "-n", str(SCRIPT)]).returncode == 0


def test_dry_run_plan(tmp_path):
    proc = run("--dry-run", "-j", "4", "recon-2018_08-ver02_31", env={"GXANA_EXTERNALS": str(tmp_path)})
    assert proc.returncode == 0, proc.stderr
    dest = f"{tmp_path}/halld_sim-recon-2018_08-ver02_31"
    assert proc.stdout.splitlines() == [
        "+ source env/setup.sh --sim=recon-2018_08-ver02_31",
        f"+ gxana externals fetch halld_sim --dest {dest}",
        f"+ cd {dest}/src && scons -u -j4 install",
    ]


def test_unknown_set():
    proc = run("--dry-run", "recon-bogus")
    assert proc.returncode == 2
    assert "unknown version set recon-bogus" in proc.stderr


def test_usage():
    assert run().returncode == 2
    assert run("--bogus", "x").returncode == 2
