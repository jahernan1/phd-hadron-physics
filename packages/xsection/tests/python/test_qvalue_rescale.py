import pytest

from gxana_xsection import qvalue_rescale


def write_pair(tmp_path, qval="60"):
    diffout = tmp_path / "diffout_a.txt"
    diffout.write_text(f"tcenter data_yield qval_yield\n0.225 50 {qval}\n0.44 80 40\n")
    diffxsec = tmp_path / "diffxsec_a.txt"
    diffxsec.write_text("tBinCenter\tdsigmadt\ttBinWidth\tYerr\n0.225  4.0  0.125  0.5\n0.44  6.0  0.09  0.4\n")
    return diffout, diffxsec


def test_process_files(tmp_path):
    diffout, diffxsec = write_pair(tmp_path)
    out = tmp_path / "out.txt"
    qvalue_rescale.process_files(str(diffout), "data_yield", "qval_yield", str(diffxsec), str(out))
    assert out.read_text() == "tBinCenter dsigmadt tBinWidth Yerr\n0.225 4.8 0.125 0.5\n0.44 3.0 0.09 0.4\n"


def test_zero_qvalue_yield_gives_zero(tmp_path):
    diffout, diffxsec = write_pair(tmp_path, qval="0")
    out = tmp_path / "o"
    qvalue_rescale.process_files(str(diffout), "data_yield", "qval_yield", str(diffxsec), str(out))
    assert out.read_text().splitlines()[1].split()[1] == "0.0"


def test_zero_data_yield_gated_bin_gives_zero_not_nan(tmp_path):
    # A bin the fit gated out has data_yield 0 (and dsigma/dt 0): the ratio is
    # taken as 0 instead of dividing by zero (legacy printed inf/NaN).
    diffout, diffxsec = write_pair(tmp_path, qval="0")
    diffout.write_text("tcenter data_yield qval_yield\n0.225 0 0\n0.44 80 40\n")
    out = tmp_path / "o"
    qvalue_rescale.process_files(str(diffout), "data_yield", "qval_yield", str(diffxsec), str(out))
    lines = out.read_text().splitlines()
    assert lines[1].split()[1] == "0.0"      # ratio 0, not inf/NaN
    assert lines[2].split()[1] == "3.0"      # other rows unchanged (40/80 * 6.0)
