"""The gxana_xsec_* executables start, print usage, and reject bad arguments."""
import subprocess

import pytest

from gxana.paths import repo_root

BIN = repo_root() / "build" / "bin"
# The channel flags gxana_xsec_tables requires (the kpkpxim values of analyses/kpkpxim/config).
CHANNEL = ["--observable", "decayxim_M", "--observable-title", "M(#Lambda#pi^{-}) (GeV/c^{2})",
           "--gate", "(hybrid_combo)*(decayxim_M>1.3&&decayxim_M<1.35)", "--qvalue-branch", "qvalue_decayxim_M",
           "--br", "0.641,0.005", "--target", "50.4,79.1,0.07008,2.01588,2"] + [
    arg for w in ("lo=1.27", "mc_hi=1.4", "mc_signal_hi=1.38", "mc_plot_hi=1.42", "data_hi=1.45", "data_edge=1.28",
                  "mcpdf_data_lo=1.275") for arg in ("--mass-window", w)]


def run(name, *args):
    exe = BIN / name
    if not exe.exists():
        pytest.skip(f"{exe} not built")
    return subprocess.run([str(exe), *args], capture_output=True, text=True)


def test_bin_help():
    out = run("gxana_xsec_bin", "--help")
    assert out.returncode == 0
    assert "usage: gxana_xsec_bin" in out.stdout


def test_bin_usage_errors():
    assert run("gxana_xsec_bin").returncode == 2
    out = run("gxana_xsec_bin", "data", "in.root", "out.root", "--energy", "6.4", "--t", "0.1,0.35",
              "--tree", "t", "--branch", "b")
    assert out.returncode == 2
    assert "at least two edges" in out.stderr
    for mode, extra, message in (("data", ["--branch", "b"], "data needs --tree"), ("mc", ["--tree", "t"], "mc needs --branch"),
                                 ("thrown", [], "thrown needs --tree")):
        out = run("gxana_xsec_bin", mode, "in.root", "out.root", "--energy", "6.4,11.4", "--t", "0.1,0.35", *extra)
        assert out.returncode == 2 and message in out.stderr


def test_bin_missing_input(tmp_path):
    out = run("gxana_xsec_bin", "thrown", str(tmp_path / "absent.root"), str(tmp_path / "o.root"),
              "--energy", "6.4,11.4", "--t", "0.1,0.35", "--tree", "t")
    assert out.returncode == 1


def test_tables_help_and_usage():
    out = run("gxana_xsec_tables", "--help")
    assert out.returncode == 0
    assert "NAME:DATA:MC:THROWN:FLUX" in out.stdout
    assert run("gxana_xsec_tables").returncode == 2
    bad = run("gxana_xsec_tables", "--fit", "Johnson", "--param", "mu=1", "--label", "l", "--out", "o", "n:d:m:t:f")
    assert bad.returncode == 2
    assert "NAME=INIT,MIN,MAX" in bad.stderr


def test_tables_missing_input(tmp_path):
    job = f"n:{tmp_path}/d.root:{tmp_path}/m.root:{tmp_path}/t.root:{tmp_path}/f.root"
    out = run("gxana_xsec_tables", "--fit", "Johnson", "--param", "mu=1.3217,1.31,1.33",
              "--weight", "w", "--label", "l", "--out", str(tmp_path / "o"), job, *CHANNEL)
    assert out.returncode == 1
    assert "cannot open" in out.stderr


def test_tables_need_the_weight_and_every_channel_flag(tmp_path):
    job = f"n:{tmp_path}/d.root:{tmp_path}/m.root:{tmp_path}/t.root:{tmp_path}/f.root"
    args = ["--fit", "Johnson", "--param", "mu=1.3217,1.31,1.33", "--out", str(tmp_path / "o")]
    out = run("gxana_xsec_tables", *args, "--label", "l", job, *CHANNEL)
    assert out.returncode == 2 and "JOB given before --weight" in out.stderr
    for i in range(0, 12, 2):  # each of the six single-valued channel flags left out in turn
        out = run("gxana_xsec_tables", *args, "--weight", "w", "--label", "l", job, *CHANNEL[:i], *CHANNEL[i + 2:])
        assert out.returncode == 2 and f"missing channel flag {CHANNEL[i]}" in out.stderr, CHANNEL[i]
    out = run("gxana_xsec_tables", *args, "--weight", "w", "--label", "l", job, *CHANNEL[:-2])
    assert out.returncode == 2 and "--mass-window needs all of" in out.stderr
    out = run("gxana_xsec_tables", *args, "--weight", "w", "--label", "l", job, *CHANNEL, "--mass-window", "hi=1")
    assert out.returncode == 2 and "unknown --mass-window hi" in out.stderr


def test_tables_job_before_label_is_usage_error(tmp_path):
    job = f"n:{tmp_path}/d.root:{tmp_path}/m.root:{tmp_path}/t.root:{tmp_path}/f.root"
    out = run("gxana_xsec_tables", "--fit", "Johnson", "--param", "mu=1.3217,1.31,1.33",
              "--out", str(tmp_path / "o"), job, "--label", "l")
    assert out.returncode == 2
    assert "before --label" in out.stderr


def test_tables_cheby_rejects_non_integer_and_out_of_range(tmp_path):
    job = f"n:{tmp_path}/d.root:{tmp_path}/m.root:{tmp_path}/t.root:{tmp_path}/f.root"
    for bad in ("1.7", "3"):
        out = run("gxana_xsec_tables", "--fit", "Johnson", "--param", "mu=1.3217,1.31,1.33",
                  "--label", "l", "--out", str(tmp_path / "o"), "--cheby", bad, job)
        assert out.returncode == 2
        assert "--cheby must be 1 or 2" in out.stderr


def test_tables_mc_shape_argument_checks(tmp_path):
    job = f"n:{tmp_path}/d.root:{tmp_path}/m.root:{tmp_path}/t.root:{tmp_path}/f.root"
    full = ["--param", "mu=1.3217,1.32,1.33", "--param", "lambda=0.004,0.002,0.007",
            "--param", "gamma=-0.01,-1,1", "--param", "delta=1.2,0.2,5"]
    out = run("gxana_xsec_tables", "--fit", "JohnsonMCShape", *full[:6], "--weight", "w",
              "--label", "hybrid_combo", "--out", str(tmp_path / "o"), job, *CHANNEL)
    assert out.returncode == 2
    assert "JohnsonMCShape needs --param delta" in out.stderr
    out = run("gxana_xsec_tables", "--fit", "JohnsonMCShape", *full, "--weight", "w",
              "--label", "hybrid_combo", "--cheby", "1", "--out", str(tmp_path / "o"), job, *CHANNEL)
    assert out.returncode == 2
    assert "JohnsonMCShape needs --cheby 2" in out.stderr
    out = run("gxana_xsec_tables", "--fit", "JohnsonMCShape", *full, "--weight", "w",
              "--label", "hybrid_combo", "--out", str(tmp_path / "o"), job, *CHANNEL)
    assert out.returncode == 1  # arguments accepted; fails opening the absent inputs
    assert "cannot open" in out.stderr


def test_tables_mc_pdf_needs_no_param(tmp_path):
    job = f"n:{tmp_path}/d.root:{tmp_path}/m.root:{tmp_path}/t.root:{tmp_path}/f.root"
    out = run("gxana_xsec_tables", "--fit", "MCPdf", "--label", "mcPdf", "--out", str(tmp_path / "o"))
    assert out.returncode == 2  # no JOB
    assert "missing arguments" in out.stderr
    out = run("gxana_xsec_tables", "--fit", "Johnson", "--weight", "w", "--label", "l", "--out", str(tmp_path / "o"),
              job, *CHANNEL)
    assert out.returncode == 2  # other fits still need --param
    assert "missing arguments" in out.stderr
    out = run("gxana_xsec_tables", "--fit", "MCPdf", "--weight", "w", "--label", "mcPdf", "--cheby", "1",
              "--out", str(tmp_path / "o"), job, *CHANNEL)
    assert out.returncode == 1  # arguments accepted; fails opening the absent inputs
    assert "cannot open" in out.stderr
