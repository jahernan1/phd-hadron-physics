"""Q-factors: the fork engine + kpkpxim run config reproduce the thesis
2017-01 values (neighbours, fit status, q-factor, chi2/ndf) on four slices
of 3 events, computed against the full preserved dataset.

GXANA_GOLDEN_QFACTORS_MODEL=<model> reruns with another configPDFs model
(used once to pick the thesis model; see config/qfactors.yaml)."""
from __future__ import annotations

import math
import os
import shutil
import subprocess
import sys
from pathlib import Path

import pytest

from gxana import config
from gxana.paths import repo_root
from gxana.stages import qfactors

sys.path.insert(0, str(repo_root() / "tests" / "qfactors"))
from qfactors_helpers import dump, entries, gx_env  # noqa: E402

pytestmark = [pytest.mark.golden,
              pytest.mark.skipif(any(shutil.which(t) is None for t in ("root", "root-config", "g++")),
                                 reason="ROOT toolchain not on PATH")]
GOLDEN = "flat_trees/postQVal_flatTree_kpkpxim__M23_2017-01_ana56_nominal_kphighrap_1111111.root"
TREE, VAR = "flatTree_kpkpxim", "decayxim_M"
Q_TOL, CHI_RTOL = 1e-3, 1e-2
THESIS_MODEL_REPRODUCED = True   # Task 7 Step 3 sets this


@pytest.fixture(scope="module")
def slices(need, tmp_path_factory):
    (src,) = need(GOLDEN)
    cfg = config.load_channel("kpkpxim")
    n = entries(src, TREE)
    nproc = n // 3
    batch = n // nproc
    tmp = tmp_path_factory.mktemp("qfgold")
    job = qfactors.plan_qfactors(cfg, "2017-01", model=os.environ.get("GXANA_GOLDEN_QFACTORS_MODEL"),
                                 input_file=src, overrides={"nProcess": nproc}, environ=gx_env(tmp))
    qfactors.stage(job)
    qfactors.compile_main(job)
    pairs = []
    for i in (0, nproc // 4, nproc // 2, 3 * nproc // 4):
        proc = subprocess.run(["./main", str(i)], cwd=job.work_dir, capture_output=True, text=True, timeout=1800)
        assert proc.returncode == 0, (proc.stdout + proc.stderr)[-4000:]
        got = dump(job.output_dir / job.combo_tag / f"results{i}.root", TREE, VAR)
        ref = dump(src, TREE, VAR, first=i * batch, count=batch)
        assert [r.entry for r in got] == [r.entry for r in ref]
        pairs += list(zip(got, ref))
    return job, pairs


def test_thesis_neighbours_reproduced(slices):
    _, pairs = slices
    bad = [(g.entry, len(set(g.neighbors) ^ set(r.neighbors))) for g, r in pairs if g.neighbors != r.neighbors]
    assert not bad, f"entries with different neighbour sets (entry, #differing): {bad}"


def test_thesis_qfactors_reproduced(slices, request):
    job, pairs = slices
    if not THESIS_MODEL_REPRODUCED:
        request.applymarker(pytest.mark.xfail(strict=True, reason=(
            "no preserved configPDFs revision reproduces the thesis q-factors")))
    table = [(g.entry, r.qvalue, g.qvalue, r.status, g.status, r.chisq, g.chisq) for g, r in pairs]
    worst = max(abs(g.qvalue - r.qvalue) for g, r in pairs)
    bad = [row for row, (g, r) in zip(table, pairs)
           if abs(g.qvalue - r.qvalue) > Q_TOL or g.status != r.status
           or (not math.isnan(r.chisq) and abs(g.chisq - r.chisq) > CHI_RTOL * abs(r.chisq))]
    assert not bad, (f"model {job.model}: max |dq| = {worst:.3g}; "
                     f"(entry, q_ref, q, status_ref, status, chi2_ref, chi2): {bad}")
