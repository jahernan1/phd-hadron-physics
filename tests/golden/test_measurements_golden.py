"""The split mass / lifetime / spin measurements reproduce the original GetXimProperties.C and
PlotGlueXSpin.C fit results on the preserved thesis trees (reference recorded from the original
macros before they were archived, see tests/golden/data/measurements_reference.txt).

Both sides run single-threaded (implicit MT off): the multithreaded histogram fill is not
bit-reproducible and the mass fit amplifies that noise, so only single-threaded runs can be pinned
tightly. The plain data tree is not preserved; the post-Q-factor tree stands in for it
(docs/KNOWN_ISSUES.md)."""
import math
import os
import shutil
import subprocess
from pathlib import Path

import pytest
from golden_data import PERIOD_TREES

from gxana.paths import repo_root

pytestmark = [pytest.mark.golden, pytest.mark.skipif(shutil.which("root") is None, reason="ROOT not on PATH")]

REFERENCE = Path(__file__).parent / "data" / "measurements_reference.txt"
MACROS = "analyses/kpkpxim/measurements"
# Every entry takes n_threads; 0 leaves implicit MT off, as in the reference run.
SEQUENCE = ("mass/PrepMass.C", "mass/FitMass.C", "lifetime/PrepLifetime.C", "lifetime/FitLifetime.C",
            "spin/PrepSpinData.C", "spin/PlotGlueXSpin.C")


def _farm(tmp_path, need):
    src = need(*[f"flat_trees/postQVal_flatTree_{s}_nominal_kphighrap_1111111.root" for s in PERIOD_TREES],
               *[f"flat_trees/flatTree_{s}_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root" for s in PERIOD_TREES],
               *[f"flat_trees/flatTree_thrown_{s}_gen_amp_V2_ac_YstarRest.root" for s in PERIOD_TREES])
    data, out = tmp_path / "data" / "flatTrees", tmp_path / "out"
    data.mkdir(parents=True)
    (out / "kpkpxim" / "prod_plots").mkdir(parents=True)
    by_name = {p.name: p for p in src}
    for s in PERIOD_TREES:
        q = by_name[f"postQVal_flatTree_{s}_nominal_kphighrap_1111111.root"]
        d = out / "kpkpxim" / "qfactors" / f"{s}_nominal_kphighrap_1111111"
        d.mkdir(parents=True)
        (d / q.name).symlink_to(q)
        (data / f"flatTree_{s}_nominal_kphighrap.root").symlink_to(q)  # stand-in for the plain data tree
        for n in (f"flatTree_{s}_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root",
                  f"flatTree_thrown_{s}_gen_amp_V2_ac_YstarRest.root"):
            (data / n).symlink_to(by_name[n])
    return tmp_path / "data", out


def _root(macro, cwd, env):
    root = repo_root()
    proc = subprocess.run(["root", "-l", "-b", "-q", str(root / "rootlogon.C"), f"{root / MACROS / macro}(0)"],
                          cwd=cwd, env=env, capture_output=True, text=True, timeout=1800)
    return proc.stdout + proc.stderr


def _parse(text):
    """FITRESULT lines -> {(kind, name, occurrence): [(key, value string), ...]}. The three mass
    periods share a name, so the n-th occurrence of (kind, name) is part of the key (period order)."""
    out, seen = {}, {}
    for line in text.splitlines():
        if not line.startswith("FITRESULT"):
            continue
        _, kind, name, *kv = line.split()
        n = seen[(kind, name)] = seen.get((kind, name), -1) + 1
        out[(kind, name, n)] = [tuple(x.split("=", 1)) for x in kv]
    return out


def _same(got, want):
    g, w = float(got), float(want)
    if not (math.isfinite(g) and math.isfinite(w)):
        return got == want
    return g == pytest.approx(w, rel=1e-9, abs=1e-12)


def test_measurements_reproduce_original(tmp_path, need):
    data, out = _farm(tmp_path, need)
    work = out / "kpkpxim" / "measurements"
    work.mkdir()
    env = dict(os.environ, GXANA_ROOT=str(repo_root()), GXANA_DATA=str(data), GXANA_OUTPUT=str(out))
    text = "".join(_root(macro, work, env) for macro in SEQUENCE)
    got, ref = _parse(text), _parse(REFERENCE.read_text())
    assert len(ref) == 13, "reference must hold 13 FITRESULT lines"
    # PlotGlueXSpin prints the merged spin line before the per-period ones (the original printed it
    # last), so lines are matched by (kind, name, occurrence), not by position.
    assert set(got) == set(ref), (
        f"missing: {sorted(set(ref) - set(got))}\nextra: {sorted(set(got) - set(ref))}\n{text[-3000:]}")
    for key, fields in ref.items():
        assert [k for k, _ in got[key]] == [k for k, _ in fields], (key, got[key], fields)
        for (k, v), (_, g) in zip(fields, got[key]):
            assert _same(g, v), (key, k, g, v)
