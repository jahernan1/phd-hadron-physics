"""gxana_xsection.syst_tables: the weighted tables with their total systematic
(the per-table output of the legacy scale-factor LaTeX tables)."""
import subprocess
import sys

import numpy as np
import pytest

from gxana_xsection import syst_tables, tex_table

HEADER = "-t d\\sigma/dt \\delta_x \\delta_y S\n"


def _tables(d, s_column=True):
    d.mkdir()
    rows = {"6.40_emax_7.40": ["0.225 4.46 0.125 0.405 0.5", "0.44 6.38 0.09 0.477 1.128"],
            "7.40_emax_7.86": ["0.225 3.9 0.125 0.38 1.2", "0.44 6.3 0.09 0.477 0.8"]}
    # written in reverse emin order: the reader sorts by emin
    for key in sorted(rows, reverse=True):
        lines = rows[key] if s_column else [" ".join(r.split()[:4]) for r in rows[key]]
        header = HEADER if s_column else "-t d\\sigma/dt \\delta_x \\delta_y\n"
        (d / f"weighted_diffxsec_emin_{key}.txt").write_text(header + "\n".join(lines) + "\n")
    return d


def _stats(path, rows):
    path.write_text("XVal XErr YMean StdDev\n" + "".join(" ".join(map(str, r)) + "\n" for r in rows))
    return str(path)


def _fit_combo(tmp_path):
    fit = _stats(tmp_path / "fit.txt", [[0.225, 0.125, 4.6, 0.17], [0.44, 0.09, 6.7, 0.28],
                                        [0.225, 0.125, 3.8, 0.12], [0.44, 0.09, 6.1, 0.2]])
    combo = _stats(tmp_path / "combo.txt", [[0.225, 0.125, 4.4, 0.05], [0.44, 0.09, 6.3, 0.1],
                                            [0.225, 0.125, 3.9, 0.0], [0.44, 0.09, 6.2, 0.03]])
    return fit, combo


def _columns(fit, combo):
    # the legacy order: the additional files, then the scale-factor run column
    return {"fit": fit, "combo": combo, "run": syst_tables.SCALE_FACTOR}


def test_total_is_the_quadrature_sum_inserted_as_fourth_column(tmp_path):
    d = _tables(tmp_path / "w")
    fit, combo = _fit_combo(tmp_path)
    written = syst_tables.write_syst_tables(str(d), _columns(fit, combo))
    assert [p.rsplit("/", 1)[-1] for p in written] == [
        "syst_weighted_diffxsec_emin_6.40_emax_7.40.txt", "syst_weighted_diffxsec_emin_7.40_emax_7.86.txt"]
    first = (d / "syst_weighted_diffxsec_emin_6.40_emax_7.40.txt").read_text().splitlines()
    assert first[0] == "-t d\\sigma/dt \\delta_x \\delta_y_syst \\delta_y S"
    assert first[1].split()[3] == f"{np.hypot(0.17, 0.05):.3f}"          # S = 0.5 < 1: no run systematic
    run = 0.477 * 1.128 - 0.477
    assert first[2].split() == ["0.44", "6.38", "0.09", f"{np.sqrt(0.28**2 + 0.1**2 + run**2):.3f}", "0.477", "1.128"]
    second = (d / "syst_weighted_diffxsec_emin_7.40_emax_7.86.txt").read_text().splitlines()
    assert second[1].split()[3] == f"{np.sqrt(0.12**2 + 0.0**2 + (0.38 * 1.2 - 0.38)**2):.3f}"


def test_matches_the_legacy_scale_factor_tables(tmp_path):
    """Byte-identical to tex_table's legacy scale-factor branch (MakeXsecTexTableScale.py)."""
    fit, combo = _fit_combo(tmp_path)
    legacy = _tables(tmp_path / "legacy")
    tex_table.process_files_to_latex(str(legacy), "weighted*.txt", r"\s+", str(tmp_path / "t.tex"),
                                     additional_files=[fit, combo], systematic_source="scale_factor")
    new = _tables(tmp_path / "new")
    syst_tables.write_syst_tables(str(new), _columns(fit, combo))
    names = sorted(p.name for p in legacy.glob("syst_*.txt"))
    assert len(names) == 2 and names == sorted(p.name for p in new.glob("syst_*.txt"))
    for name in names:
        assert (new / name).read_text() == (legacy / name).read_text(), name


def test_rerun_overwrites_and_out_dir(tmp_path):
    d = _tables(tmp_path / "w")
    fit, combo = _fit_combo(tmp_path)
    syst_tables.write_syst_tables(str(d), _columns(fit, combo))
    syst_tables.write_syst_tables(str(d), _columns(fit, combo))
    assert len(list(d.glob("syst_*.txt"))) == 2
    out = tmp_path / "elsewhere"
    syst_tables.write_syst_tables(str(d), {"fit": fit}, out_dir=str(out))
    assert sorted(p.name for p in out.iterdir()) == [
        "syst_weighted_diffxsec_emin_6.40_emax_7.40.txt", "syst_weighted_diffxsec_emin_7.40_emax_7.86.txt"]


def test_tex_ignores_the_systematic_tables(tmp_path):
    """The tex step globs weighted*.txt in the same directory: syst_* must not change its table."""
    fit, combo = _fit_combo(tmp_path)
    d = _tables(tmp_path / "w")
    before = tmp_path / "before.tex"
    assert tex_table.process_files_to_latex(str(d), "weighted*.txt", r"\s+", str(before),
                                            columns={"fit": fit, "combo": combo}) is not None
    syst_tables.write_syst_tables(str(d), _columns(fit, combo))
    after = tmp_path / "after.tex"
    tex_table.process_files_to_latex(str(d), "weighted*.txt", r"\s+", str(after), columns={"fit": fit, "combo": combo})
    assert after.read_text() == before.read_text()


def test_stats_file_on_other_t_points_is_an_error(tmp_path):
    d = _tables(tmp_path / "w")
    bad = _stats(tmp_path / "bad.txt", [[0.225, 0.125, 4.6, 0.17], [0.53, 0.09, 6.7, 0.28],
                                        [0.225, 0.125, 3.8, 0.12], [0.44, 0.09, 6.1, 0.2]])
    with pytest.raises(ValueError, match=r"bad\.txt.*XVal"):
        syst_tables.write_syst_tables(str(d), {"bad": bad})


def test_stats_file_row_count_must_match(tmp_path):
    d = _tables(tmp_path / "w")
    short = _stats(tmp_path / "short.txt", [[0.225, 0.125, 4.6, 0.17], [0.44, 0.09, 6.7, 0.28]])
    with pytest.raises(ValueError, match="fewer rows"):
        syst_tables.write_syst_tables(str(d), {"short": short})
    fit, _ = _fit_combo(tmp_path)
    long_ = _stats(tmp_path / "long.txt", [[0.225, 0.125, 4.6, 0.17], [0.44, 0.09, 6.7, 0.28],
                                           [0.225, 0.125, 3.8, 0.12], [0.44, 0.09, 6.1, 0.2], [1.0, 0.1, 1.0, 0.1]])
    with pytest.raises(ValueError, match=r"long\.txt has 5 rows, the tables 4"):
        syst_tables.write_syst_tables(str(d), {"fit": fit, "long": long_})


def test_scale_factor_needs_the_s_column(tmp_path):
    d = _tables(tmp_path / "w", s_column=False)
    with pytest.raises(ValueError, match="S column"):
        syst_tables.write_syst_tables(str(d), {"run": syst_tables.SCALE_FACTOR})


def test_no_column_or_no_table_is_an_error(tmp_path):
    d = _tables(tmp_path / "w")
    with pytest.raises(ValueError, match="at least one"):
        syst_tables.write_syst_tables(str(d), {})
    with pytest.raises(FileNotFoundError, match=r"nothing\*\.txt"):
        syst_tables.write_syst_tables(str(tmp_path / "w"), {"run": "scale_factor"}, pattern="nothing*.txt")


def _cli(*args):
    return subprocess.run([sys.executable, "-m", "gxana_xsection.syst_tables", *args],
                          capture_output=True, text=True)


def test_cli(tmp_path):
    d = _tables(tmp_path / "w")
    fit, combo = _fit_combo(tmp_path)
    proc = _cli(str(d), "--column", f"fit={fit}", "--column", f"combo={combo}", "--column", "run=scale_factor")
    assert proc.returncode == 0, proc.stderr
    api = _tables(tmp_path / "api")
    syst_tables.write_syst_tables(str(api), _columns(fit, combo))
    for path in api.glob("syst_*.txt"):
        assert (d / path.name).read_text() == path.read_text()
    missing = _cli(str(tmp_path / "nope"), "--column", "run=scale_factor")
    assert missing.returncode == 1 and "error" in missing.stderr
    assert _cli(str(d), "--column", "fit").returncode == 2


def test_scale_factor_is_defined_once():
    """Other code imports syst_tables.SCALE_FACTOR; a second literal definition must agree."""
    import pathlib
    import re
    root = pathlib.Path(__file__).resolve().parents[4]
    for path in list((root / "packages").rglob("*.py")) + list((root / "analyses").rglob("*.py")):
        if ".venv" in path.parts or "_workdir" in path.parts:
            continue
        for value in re.findall(r"^SCALE_FACTOR\s*=\s*[\"']([^\"']*)[\"']", path.read_text(), re.M):
            assert value == syst_tables.SCALE_FACTOR, path
