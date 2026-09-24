import math

import pytest

from gxana_xsection import compare


def write(path, text):
    path.write_text(text)
    return path


def test_identical_tables(tmp_path):
    a = write(tmp_path / "a.txt", "tcenter\tterr\n0.225  0.125\n0.44  nan\n")
    b = write(tmp_path / "b.txt", "tcenter terr\n0.225 0.125\n0.44 nan\n")
    result = compare.compare_tables(a, b)
    assert result.problems == []
    assert result.max_rel == 0.0


def test_relative_tolerance(tmp_path):
    a = write(tmp_path / "a.txt", "x\n1.0001\n")
    b = write(tmp_path / "b.txt", "x\n1.0\n")
    assert compare.compare_tables(a, b, rtol=1e-3).problems == []
    result = compare.compare_tables(a, b, rtol=1e-5)
    assert len(result.problems) == 1
    assert result.problems[0].startswith("line 2 col 1: 1.0001 vs 1.0")
    assert result.max_rel == pytest.approx(1e-4, rel=1e-3)


def test_header_shape_and_nan_mismatch(tmp_path):
    a = write(tmp_path / "a.txt", "x y\n1 2\n3 nan\n")
    b = write(tmp_path / "b.txt", "x z\n1 2\n3 4\n4 5\n")
    problems = compare.compare_tables(a, b).problems
    assert "3 rows vs 4 in reference" in problems
    assert "line 1 col 2: 'y' vs 'z'" in problems
    assert any(p.startswith("line 3 col 2: nan vs 4") for p in problems)


def test_inf_matches_inf(tmp_path):
    a = write(tmp_path / "a.txt", "inf -inf\n")
    b = write(tmp_path / "b.txt", "inf -inf\n")
    assert compare.compare_tables(a, b).problems == []


def test_compare_dirs(tmp_path):
    new, ref = tmp_path / "new", tmp_path / "ref"
    new.mkdir()
    ref.mkdir()
    write(new / "a.txt", "1\n")
    write(ref / "a.txt", "1\n")
    write(ref / "c.txt", "1\n")
    assert not compare.compare_dirs(new, ref).ok
    report = compare.compare_dirs(new, ref, only_new=True)
    assert report.ok
    assert report.unproduced == ["c.txt"]
    write(new / "b.txt", "1\n")
    report = compare.compare_dirs(new, ref, only_new=True)
    assert not report.ok
    assert report.unreferenced == ["b.txt"]
    assert "no reference for: b.txt" in report.summary()


def test_cli(tmp_path, capsys):
    new, ref = tmp_path / "new", tmp_path / "ref"
    new.mkdir()
    ref.mkdir()
    write(new / "a.txt", "1.5\n")
    write(ref / "a.txt", "1.0\n")
    assert compare.main([str(new), str(ref)]) == 1
    assert "a.txt: line 1 col 1" in capsys.readouterr().out
    assert compare.main([str(new), str(ref), "--rtol", "0.5"]) == 0


def test_column_count_mismatch_is_not_reported_as_zero_deviation(tmp_path):
    import math
    (tmp_path / "new").mkdir(); (tmp_path / "ref").mkdir()
    (tmp_path / "new" / "t.txt").write_text("a\tb\n1\t2\n")
    (tmp_path / "ref" / "t.txt").write_text("a\tb\tc\n1\t2\t3\n")
    report = compare.compare_dirs(tmp_path / "new", tmp_path / "ref", rtol=1e-6)
    assert not report.ok
    assert "columns" in report.summary()
    assert math.isinf(report.max_rel)
