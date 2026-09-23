import numpy as np
import pytest

from gxana_xsection import weighted_average as wa


def test_calculate_weighted_average():
    y = np.array([[2.0, 1.0], [4.0, 1.0]])
    e = np.array([[1.0, 0.5], [1.0, 0.5]])
    avg, err, scale = wa.calculate_weighted_average(y, e)
    assert avg == pytest.approx([3.0, 1.0])
    assert err == pytest.approx([1 / np.sqrt(2), 0.5 / np.sqrt(2)])
    assert scale == pytest.approx([np.sqrt(2.0), 0.0])


@pytest.mark.filterwarnings("ignore:divide by zero:RuntimeWarning")
def test_zero_error_point_gets_no_weight():
    y = np.array([[2.0], [4.0], [9.0]])
    e = np.array([[1.0], [1.0], [0.0]])
    avg, err, scale = wa.calculate_weighted_average(y, e)
    assert avg == pytest.approx([3.0])
    assert scale == pytest.approx([np.sqrt(2.0)])


def test_create_output_filename():
    diff = [f"/d/diffxsec_flatTree_kpkpxim__{p}_emin_6.40_emax_7.40.txt"
            for p in ("M23_2017-01_ana56", "B4_M23_2018-01_ana03")]
    assert wa.create_output_filename(diff, "/out") == "/out/weighted_diffxsec_emin_6.40_emax_7.40.txt"
    tot = ["/d/totxsec_flatTree_kpkpxim__M23_2017-01_ana56.txt", "/d/totxsec_flatTree_kpkpxim__B4_M23_2018-01_ana03.txt"]
    assert wa.create_output_filename(tot, "/out") == "/out/totxsec_weighted_output.txt"


def test_weight_files_end_to_end(tmp_path):
    src = tmp_path / "src"
    src.mkdir()
    for i, y in enumerate((2.0, 4.0, 3.0)):
        (src / f"diffxsec_P{i}_emin_6.40_emax_7.40.txt").write_text(
            f"tBinCenter\tdsigmadt\ttBinWidth\tYerr\n0.225  {y}  0.125  1.0\n0.44  {y}  0.09  1.0\n")
    out = wa.weight_files(str(src), str(tmp_path), pattern="diffxsec*.txt")
    assert out == str(tmp_path / "weighted_diffxsec_emin_6.40_emax_7.40.txt")
    assert (tmp_path / "weighted_diffxsec_emin_6.40_emax_7.40.txt").read_text() == (
        "-t d\\sigma/dt \\delta_x \\delta_y S\n"
        "0.225000 3.000000 0.125000 0.577350 1.000000\n"
        "0.440000 3.000000 0.090000 0.577350 1.000000\n"
    )


def test_weight_files_needs_three_periods(tmp_path):
    (tmp_path / "diffxsec_a.txt").write_text("x y ex ey\n1 2 3 4\n")
    with pytest.raises(ValueError, match="expected 3"):
        wa.weight_files(str(tmp_path), str(tmp_path), pattern="diffxsec*.txt")


def test_no_matching_files(tmp_path):
    with pytest.raises(FileNotFoundError):
        wa.get_files_from_directory(str(tmp_path), pattern="none*.txt")
