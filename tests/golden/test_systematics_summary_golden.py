"""Golden: the summary total reproduces the published delta-y(syst) column of
diffxsec_table_scale.tex in all 56 bins."""
import re

import numpy as np
import pytest

from gxana_systematics import sfactor, spread, summary
from gxana_systematics.tables import read_periods, read_weighted

pytestmark = pytest.mark.golden
REF = "reference/xsection"


def test_summary_total_reproduces_published_syst_column(need, tmp_path):
    per, weighted, tables = need(f"{REF}/johnson", f"{REF}/weighted/johnson", f"{REF}/tables")
    rows = []
    for block in read_weighted(str(weighted)):
        periods = read_periods(str(per), block.emin, block.emax, 3)
        r = sfactor.scale_factor(np.array([p[:, 1] for p in periods]), np.array([p[:, 3] for p in periods]))
        syst = sfactor.run_systematic(r.stat_err, r.s)
        rows += [(p0[0], p0[2], m, e, c, n, s, y) for p0, m, e, c, n, s, y in
                 zip(periods[0], r.mean, r.stat_err, r.chi2, r.n, r.s, syst)]
    run = tmp_path / "sfactor_stats.txt"
    spread.write_stats(str(run), rows, header=sfactor.HEADER)
    out = tmp_path / "summary"
    assert summary.main(["--nominal-dir", str(weighted), "--out-dir", str(out), "--column", f"run={run}",
                         "--column", f"accidentals={tables / 'combo_variations_stats.txt'}",
                         "--column", f"fit={tables / 'fit_variations_stats.txt'}",
                         "--normalization", "luminosity=0.05"]) == 0
    total = np.loadtxt(out / "systematics_summary.txt", skiprows=1, usecols=-1)
    published = []
    for line in (tables / "diffxsec_table_scale.tex").read_text().splitlines():
        fields = [f.strip() for f in line.rstrip("\\ ").split("&")]
        if len(fields) >= 5 and re.fullmatch(r"\(\d+\.\d+, \d+\.\d+\)", fields[-4]):
            published.append(float(fields[-1]))
    assert len(published) == len(total) == 56
    np.testing.assert_array_equal(np.round(total, 3), published)
    assert (out / "normalization.txt").read_text() == "luminosity 0.05\n"
