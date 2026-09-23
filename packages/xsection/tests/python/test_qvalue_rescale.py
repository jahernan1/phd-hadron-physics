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


def test_zero_qvalue_yield_rejected(tmp_path):
    diffout, diffxsec = write_pair(tmp_path, qval="0")
    with pytest.raises(ValueError, match="Denominator"):
        qvalue_rescale.process_files(str(diffout), "data_yield", "qval_yield", str(diffxsec), str(tmp_path / "o"))


def test_process_files_in_directory(tmp_path):
    write_pair(tmp_path)
    out = tmp_path / "out"
    qvalue_rescale.process_files_in_directory(str(tmp_path), "diffout*.txt", "diffxsec*.txt", str(out))
    assert [p.name for p in out.iterdir()] == ["diffxsec_a.txt"]
