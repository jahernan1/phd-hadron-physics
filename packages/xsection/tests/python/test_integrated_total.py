import numpy as np
import pytest

from gxana_xsection import integrated_total as it


def _write(path, rows):
    lines = ["tBinCenter\tdsigmadt\ttBinWidth\tYerr"] + ["  ".join(str(v) for v in r) for r in rows]
    path.write_text("\n".join(lines) + "\n")


def test_integrate_table_non_uniform_widths():
    # full widths 0.5 and 1.0
    sigma, err = it.integrate_table([2.0, 3.0], [0.25, 0.5], [0.2, 0.1])
    assert sigma == pytest.approx(2.0 * 0.5 + 3.0 * 1.0)
    assert err == pytest.approx(np.hypot(0.2 * 0.5, 0.1 * 1.0))


def test_gated_zero_row_contributes_zero():
    with_zero = it.integrate_table([2.0, 0.0], [0.25, 0.5], [0.2, 0.0])
    without = it.integrate_table([2.0], [0.25], [0.2])
    assert with_zero == pytest.approx(without)


def test_integrate_period_writes_one_row_per_energy_bin_sorted(tmp_path):
    name = "flatTree_x__A"
    # written out of order: 7.40-7.86 first
    _write(tmp_path / f"diffxsec_{name}_emin_7.40_emax_7.86.txt", [(0.2, 1.0, 0.1, 0.1), (0.5, 0.0, 0.2, 0.0)])
    _write(tmp_path / f"diffxsec_{name}_emin_6.40_emax_7.40.txt", [(0.2, 2.0, 0.1, 0.3), (0.5, 4.0, 0.2, 0.4)])
    _write(tmp_path / "diffxsec_flatTree_x__B_emin_6.40_emax_7.40.txt", [(0.2, 9.0, 0.1, 0.9)])
    out = tmp_path / "out"
    path = it.integrate_period(str(tmp_path), name, str(out))
    assert path == str(out / f"intxsec_{name}.txt")
    text = (out / f"intxsec_{name}.txt").read_text().splitlines()
    assert text[0] == "enBinCenter\tsigma\tenBinWidth\tYerr"
    rows = np.loadtxt(path, skiprows=1)
    assert rows.shape == (2, 4)
    assert rows[0] == pytest.approx([6.9, 2.0 * 0.2 + 4.0 * 0.4, 0.5, np.hypot(0.3 * 0.2, 0.4 * 0.4)], abs=1e-6)
    assert rows[1] == pytest.approx([7.63, 1.0 * 0.2, 0.23, 0.1 * 0.2], abs=1e-6)


def test_integrate_directory_discovers_names(tmp_path):
    for n in ("flatTree_x__A", "flatTree_x__B"):
        _write(tmp_path / f"diffxsec_{n}_emin_6.40_emax_7.40.txt", [(0.2, 1.0, 0.5, 0.1)])
    _write(tmp_path / "totxsec_flatTree_x__A.txt", [(6.9, 1.0, 0.5, 0.1)])
    assert it.discover_names(str(tmp_path)) == ["flatTree_x__A", "flatTree_x__B"]
    paths = it.integrate_directory(str(tmp_path), str(tmp_path / "o"))
    assert [p.rsplit("/", 1)[1] for p in paths] == ["intxsec_flatTree_x__A.txt", "intxsec_flatTree_x__B.txt"]
    assert len(it.integrate_directory(str(tmp_path), str(tmp_path / "o2"), name="flatTree_x__B")) == 1


def test_missing_files_raise(tmp_path):
    with pytest.raises(FileNotFoundError):
        it.integrate_period(str(tmp_path), "nope", str(tmp_path))
    with pytest.raises(FileNotFoundError):
        it.integrate_directory(str(tmp_path), str(tmp_path))


def test_output_is_readable_by_weighted_average(tmp_path):
    from gxana_xsection import weighted_average as wa
    for i, n in enumerate(("flatTree_x__A", "flatTree_x__B", "flatTree_x__C")):
        _write(tmp_path / f"diffxsec_{n}_emin_6.40_emax_7.40.txt", [(0.2, 1.0 + i, 0.5, 0.1)])
        it.integrate_period(str(tmp_path), n, str(tmp_path))
    (tmp_path / "w").mkdir()
    out = wa.weight_files(str(tmp_path), str(tmp_path / "w"), pattern="intxsec*.txt")
    assert out.endswith("intxsec_weighted_output.txt")
