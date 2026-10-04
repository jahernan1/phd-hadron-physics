"""End-to-end Q-factor run on a toy tree through the real stage and run.py.

2 processes x 300 events, kDim 40, one drawn event per process. Covers
the fork's chiSqNdf (NaN when not drawn), fitParams / paramLog alignment,
mergeQresults carry-over and makePlots (variable file, signal sum).
On macOS run.py's GNU `sed -i` calls fail silently; the stage has already
rendered identical values, so the run is the same as on Linux."""
import re
from typing import cast
import shutil
import subprocess
from pathlib import Path

import pytest

from gxana import config
from gxana.stages import qfactors

from qfactors_helpers import dump, gx_env, is_nan, make_toy_tree

CFG = config.load_channel("kpkpxim")
ROOT = Path(__file__).resolve().parents[2]
pytestmark = [
    pytest.mark.skipif(not (ROOT / "packages/qfactors/main.C").is_file(), reason="packages/qfactors not checked out"),
    pytest.mark.skipif(any(shutil.which(t) is None for t in ("root", "root-config", "hadd", "g++")),
                       reason="ROOT toolchain not on PATH"),
]
N = 600


@pytest.fixture(scope="module")
def run(tmp_path_factory):
    tmp = tmp_path_factory.mktemp("qf")
    env = gx_env(tmp)
    toy = tmp / "toy.root"
    make_toy_tree(toy, N)
    job = qfactors.plan_qfactors(CFG, "2017-01", input_file=toy, environ=env,
                                 overrides={"nProcess": 2, "kDim": 40, "numberEventsToSavePerProcess": 1},
                                 diagnostic_vars=["decayxim_M", "beam_E"])
    captured = []

    def runner(argv, **kw):
        result = subprocess.run(argv, capture_output=True, text=True, timeout=900, **kw)
        captured.append(result)
        return result

    rc = qfactors.run_qfactors(job, ["fit", "plots"], environ=env, runner=runner, log=lambda s: None)
    out = "".join(r.stdout + r.stderr for r in captured)
    assert rc == 0, out[-4000:]
    return job, out


def test_postqval_has_a_qfactor_for_every_event(run):
    job, _ = run
    rows = dump(job.result, "flatTree_kpkpxim", "decayxim_M")
    assert len(rows) == N
    assert all(0.0 <= r.qvalue <= 1.0 for r in rows)


def test_chisq_is_nan_except_for_drawn_events(run):
    job, _ = run
    rows = dump(job.result, "flatTree_kpkpxim", "decayxim_M")
    drawn = [r for r in rows if not is_nan(r.chisq)]
    assert len(drawn) == 2, [r.entry for r in drawn]   # numberEventsToSavePerProcess=1, 2 processes
    assert all(r.chisq > 0 for r in drawn)


def test_param_names_align_across_branch_log_and_values(run):
    job, _ = run
    log = (job.output_dir / job.combo_tag / "paramLog0.txt").read_text().splitlines()
    header = re.fullmatch(r"#\((.*)\)", log[0])
    assert header, log[0]
    names = header.group(1).split(",")
    assert "nsig" in names and "nbkg" in names      # no per-process suffix
    data = [l for l in log[1:] if l.strip()]
    assert len(data) == N // 2                          # one fit per event (nBS=0, no redistribution)
    assert all(len(l.split()) == len(names) for l in data), data[0]
    for i in (0, 1):                                    # same leaf names in every process
        leaves = subprocess.run(
            ["root", "-l", "-b", "-q", "-e",
             f'TFile f("{job.output_dir / job.combo_tag / f"results{i}.root"}"); '
             'auto b=((TTree*)f.Get("flatTree_kpkpxim"))->GetBranch("fitParams_decayxim_M"); '
             'for (auto l: *b->GetListOfLeaves()) printf("LEAF %s\\n", l->GetName());'],
            capture_output=True, text=True, timeout=300).stdout
        assert [l.split()[1] for l in leaves.splitlines() if l.startswith("LEAF ")] == names, i


def test_makeplots_used_var_file_and_printed_signal_sum(run):
    job, out = run
    assert "Read 2 variables from makePlotsVars.txt" in out
    total = float(cast(re.Match, re.search(r"NUMBER OF SIGNAL EVENTS: ([0-9.eE+-]+)", out)).group(1))
    assert 0 < total < N
    assert (job.plots_dir / "diagnosticPlots" / job.combo_tag / f"postQVal_hists_{job.combo_tag}.root").is_file()
