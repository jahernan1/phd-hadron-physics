import pytest

from gxana_xsection import components


def test_split_text_file(tmp_path):
    src = tmp_path / "diffout_flatTree_kpkpxim__P_emin_6.40_emax_7.40.txt"
    src.write_text("tcenter\tterr\tdata_yield\tyield_err\taccept\taccep_err\n"
                   "0.225  0.125  57.9752  8.15258  0.00376804  0.000133914\n")
    out = tmp_path / "out"
    out.mkdir()
    components.split_text_file(str(src), str(out), "kpkpxim")
    assert sorted(p.name for p in out.iterdir()) == [
        "accept_kpkpxim__P_emin_6.40_emax_7.40.txt",
        "data_yield_kpkpxim__P_emin_6.40_emax_7.40.txt",
    ]
    assert (out / "accept_kpkpxim__P_emin_6.40_emax_7.40.txt").read_text() == (
        "#tcenter accept terr accep_err\n0.225 0.00376804 0.125 0.000133914\n")


def test_split_files_by_pattern(tmp_path):
    for period in ("2017-01", "2018-01"):
        (tmp_path / f"totout_flatTree_kpkpxim__{period}.txt").write_text("e ee y ey\n6.9 0.5 1 2\n")
    out = tmp_path / "out"
    out.mkdir()
    components.split_files(str(tmp_path), str(out), pattern="totout*2017-01*.txt", anchor="kpkpxim")
    assert [p.name for p in out.iterdir()] == ["y_kpkpxim__2017-01.txt"]


def test_odd_column_count_rejected(tmp_path):
    src = tmp_path / "diffout_kpkpxim_x.txt"
    src.write_text("a b c\n1 2 3\n")
    with pytest.raises(ValueError, match="Invalid file structure"):
        components.split_text_file(str(src), str(tmp_path), "kpkpxim")


def test_a_name_without_the_anchor_collapses(tmp_path):
    """Legacy slicing, kept: name[name.find(anchor):] with find() == -1 keeps the last character."""
    (tmp_path / "totout_flatTree_other__2017-01.txt").write_text("e ee y ey\n6.9 0.5 1 2\n")
    out = tmp_path / "out"
    out.mkdir()
    components.split_files(str(tmp_path), str(out), pattern="totout*.txt", anchor="kpkpxim")
    assert [p.name for p in out.iterdir()] == ["y_1.txt"]


def test_anchor_is_required():
    with pytest.raises(SystemExit):
        components.main(["in", "out", "--pattern", "x"])
