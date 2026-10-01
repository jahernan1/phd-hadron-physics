from pathlib import Path

import numpy as np
import pytest

from gxana_systematics import tables

HEADER = "-t d\\sigma/dt \\delta_x \\delta_y S\n"


def _weighted(d: Path, emin: str, emax: str, rows):
    d.mkdir(parents=True, exist_ok=True)
    body = "".join(" ".join(f"{v:.6f}" for v in r) + "\n" for r in rows)
    (d / f"weighted_diffxsec_emin_{emin}_emax_{emax}.txt").write_text(HEADER + body)


def test_read_weighted_sorts_by_energy_and_ignores_derived_files(tmp_path):
    _weighted(tmp_path, "10.18", "11.40", [[0.2, 1.0, 0.1, 0.1, 1.0]])
    _weighted(tmp_path, "6.40", "7.40", [[0.2, 2.0, 0.1, 0.1, 1.0]])
    (tmp_path / "syst_weighted_diffxsec_emin_6.40_emax_7.40.txt").write_text("junk\n")
    blocks = tables.read_weighted(str(tmp_path))
    assert [(b.emin, b.emax) for b in blocks] == [("6.40", "7.40"), ("10.18", "11.40")]
    assert blocks[0].rows.shape == (1, 5) and blocks[0].rows[0, 1] == 2.0


def test_read_weighted_empty_dir_raises(tmp_path):
    with pytest.raises(tables.TableError, match="weighted_diffxsec"):
        tables.read_weighted(str(tmp_path))


def test_read_periods_requires_period_count(tmp_path):
    for stem in ("a", "b"):
        (tmp_path / f"diffxsec_flatTree_{stem}_emin_6.40_emax_7.40.txt").write_text(
            "tBinCenter dsigmadt tBinWidth Yerr\n0.2 1 0.1 0.5\n")
    assert len(tables.read_periods(str(tmp_path), "6.40", "7.40", 2)) == 2
    with pytest.raises(tables.TableError, match="expected 3"):
        tables.read_periods(str(tmp_path), "6.40", "7.40", 3)


def test_same_tables_reports_differences(tmp_path):
    _weighted(tmp_path / "a", "6.40", "7.40", [[0.2, 1.0, 0.1, 0.1, 1.0]])
    _weighted(tmp_path / "b", "6.40", "7.40", [[0.2, 1.0, 0.1, 0.1, 1.0]])
    assert tables.same_tables(str(tmp_path / "a"), str(tmp_path / "b")) == []
    _weighted(tmp_path / "b", "6.40", "7.40", [[0.2, 1.5, 0.1, 0.1, 1.0]])
    assert tables.same_tables(str(tmp_path / "a"), str(tmp_path / "b")) == [
        "weighted_diffxsec_emin_6.40_emax_7.40.txt"]


def test_same_tables_cli_exit_code(tmp_path, capsys):
    _weighted(tmp_path / "a", "6.40", "7.40", [[0.2, 1.0, 0.1, 0.1, 1.0]])
    _weighted(tmp_path / "b", "6.40", "7.40", [[0.2, 2.0, 0.1, 0.1, 1.0]])
    assert tables.main(["--same", str(tmp_path / "a"), str(tmp_path / "b")]) == 1
    assert "weighted_diffxsec_emin_6.40_emax_7.40.txt" in capsys.readouterr().out
