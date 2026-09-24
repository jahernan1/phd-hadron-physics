import importlib.util
import shutil
from pathlib import Path

import pytest

from gxana.paths import analysis_data_root
from gxana_xsection import tex_table

LEGACY = Path(__file__).resolve().parents[4] / "_workdir" / "AnalysisNote" / "xsection"


def _legacy(name):
    if not (LEGACY / name).exists():
        pytest.skip("legacy _workdir not present")
    spec = importlib.util.spec_from_file_location(name[:-3], LEGACY / name)
    mod = importlib.util.module_from_spec(spec)
    # The legacy scripts call process_files_to_latex(...) at module level on a
    # nonexistent example directory, but that function catches every
    # exception internally and just prints "Error: ...", so exec'ing the
    # module at import time is safe (it never raises, never touches real data).
    spec.loader.exec_module(mod)
    return mod


@pytest.fixture
def fixture_dir(tmp_path):
    # Shaped like the real weighted_average output columns (see
    # gxana_xsection.weighted_average.HEADER): -t, dsigma/dt, delta_x, delta_y, S.
    d = tmp_path / "data"
    d.mkdir()
    (d / "diffxsec_test_emin_6.40_emax_7.40.txt").write_text(
        "-t\tdsigmadt\tdelta_x\tdelta_y\tS\n"
        "0.10\t4.0\t0.05\t0.30\t0.80\n"
        "0.35\t5.0\t0.05\t0.40\t1.20\n"
        "0.53\t6.0\t0.05\t0.50\t0.95\n"
    )
    return d


@pytest.fixture
def additional_files(tmp_path):
    accidentals = tmp_path / "fit_variations_stats.txt"
    accidentals.write_text("label\tval\nA\t0.10\nB\t0.20\nC\t0.15\n")
    yield_extraction = tmp_path / "combo_variations_stats.txt"
    yield_extraction.write_text("label\tval\nA\t0.05\nB\t0.07\nC\t0.06\n")
    return [str(accidentals), str(yield_extraction)]


OPTIONS = {
    "MakeXsecTexTable1.py": {},
    "MakeXsecTexTable.py": {},
    "MakeXsecTexTableScale.py": {"systematic_source": "scale_factor"},
}


@pytest.mark.parametrize("script,with_additional", [("MakeXsecTexTable1.py", False),
                                                     ("MakeXsecTexTable.py", True),
                                                     ("MakeXsecTexTableScale.py", True)])
def test_matches_legacy(tmp_path, script, with_additional, fixture_dir, additional_files):
    legacy = _legacy(script)
    ref_out, new_out = tmp_path / "ref.tex", tmp_path / "new.tex"
    args = (str(fixture_dir), "diffxsec*", "\t")
    if with_additional:
        legacy.process_files_to_latex(*args, additional_files, str(ref_out))
        tex_table.process_files_to_latex(*args, str(new_out), additional_files=additional_files, **OPTIONS[script])
    else:
        legacy.process_files_to_latex(*args, str(ref_out))
        tex_table.process_files_to_latex(*args, str(new_out), **OPTIONS[script])
    assert new_out.read_text() == ref_out.read_text()


def test_syst_table_written_relative_to_cwd(tmp_path, fixture_dir, additional_files, monkeypatch):
    """The companion syst_<output_file> table is written by prepending "syst_" to
    output_file verbatim (a legacy quirk, preserved as-is): with a relative
    output_file this lands next to it; matches legacy byte-for-byte."""
    legacy = _legacy("MakeXsecTexTableScale.py")
    monkeypatch.chdir(tmp_path)
    args = (str(fixture_dir), "diffxsec*", "\t")
    legacy.process_files_to_latex(*args, additional_files, "ref.tex")
    tex_table.process_files_to_latex(*args, "new.tex", additional_files=additional_files,
                                      systematic_source="scale_factor")
    assert (tmp_path / "syst_new.tex").read_text() == (tmp_path / "syst_ref.tex").read_text()


def test_matches_legacy_on_preserved_data(tmp_path):
    """Golden-style equivalence: run each legacy script with its own __main__
    parameters (pattern="weighted*.txt", delimiter="\\s+") against the real
    preserved per-run-period-weighted tables (weighted/johnson/), and assert
    the new tex_table output is byte-identical. Skips when the preserved
    data or _workdir is absent.

    The legacy __main__'s additional_files ("fit_variations_stats.txt",
    "combo_variations_stats.txt") are not themselves in the preserved data,
    so the two systematic-source additional files are built from real
    per-bin columns (delta_x, S) of the preserved weighted tables
    concatenated in file order, rather than fabricated numbers.
    """
    src = analysis_data_root() / "kpkpxim" / "reference" / "xsection" / "weighted" / "johnson"
    if not src.is_dir():
        pytest.skip(f"no preserved data at {src}")
    primary = sorted(src.glob("weighted_diffxsec_emin_*.txt"))
    if not primary:
        pytest.skip(f"no weighted_diffxsec_emin_*.txt under {src}")

    def _extract_column(col_index):
        values = []
        for p in primary:
            lines = p.read_text().strip().splitlines()[1:]
            values.extend(line.split()[col_index] for line in lines)
        return values

    delta_x_values = _extract_column(2)
    scale_values = _extract_column(4)

    def _write_additional(name, values):
        path = tmp_path / name
        path.write_text("val\n" + "\n".join(values) + "\n")
        return str(path)

    additional_files = [
        _write_additional("fit_variations_stats.txt", delta_x_values),
        _write_additional("combo_variations_stats.txt", scale_values),
    ]

    cases = [
        ("MakeXsecTexTable1.py", None),
        ("MakeXsecTexTable.py", {}),
        ("MakeXsecTexTableScale.py", {"systematic_source": "scale_factor"}),
    ]
    for script, kwargs in cases:
        legacy = _legacy(script)

        legacy_dir = tmp_path / f"legacy_{script}"
        new_dir = tmp_path / f"new_{script}"
        legacy_dir.mkdir()
        new_dir.mkdir()
        for p in primary:
            shutil.copy(p, legacy_dir / p.name)
            shutil.copy(p, new_dir / p.name)

        ref_out = tmp_path / f"ref_{script}.tex"
        new_out = tmp_path / f"new_{script}.tex"
        args_legacy = (str(legacy_dir), "weighted*.txt", r"\s+")
        args_new = (str(new_dir), "weighted*.txt", r"\s+")
        if kwargs is None:
            legacy.process_files_to_latex(*args_legacy, str(ref_out))
            tex_table.process_files_to_latex(*args_new, str(new_out))
        else:
            legacy.process_files_to_latex(*args_legacy, additional_files, str(ref_out))
            tex_table.process_files_to_latex(*args_new, str(new_out), additional_files=additional_files, **kwargs)

        assert new_out.read_text() == ref_out.read_text(), script


def test_process_files_to_latex_no_additional_files_literal(tmp_path):
    """Pure unit test (no _workdir dependency): 2-row fixture, expected LaTeX written out literally."""
    d = tmp_path / "data"
    d.mkdir()
    (d / "diffxsec_test_emin_6.40_emax_7.40.txt").write_text(
        "-t\tdsigmadt\tdelta_x\tdelta_y\tS\n"
        "0.10\t4.0\t0.05\t0.30\t0.80\n"
        "0.35\t5.0\t0.05\t0.40\t1.20\n"
    )
    out = tmp_path / "table.tex"
    result = tex_table.process_files_to_latex(str(d), "diffxsec*", "\t", str(out))

    expected = (
        "\\begin{longtable}{c|c|cc}\n"
        "\\label{tab:diffxsec} \\\\\n"
        "\\toprule\n"
        "$E_\\gamma\\ (\\text{GeV})$ & $-t\\ (\\text{GeV}^2)$ & $d\\sigma/dt\\ (\\text{nb/GeV}^2)$ & $\\delta y (stat)$ \\\\\n"
        "\\midrule\n"
        "\\endfirsthead\n"
        "\\toprule\n"
        "$E_\\gamma\\ (\\text{GeV})$ & $-t\\ (\\text{GeV}^2)$ & $d\\sigma/dt\\ (\\text{nb/GeV}^2)$ & $\\delta y (stat)$ \\\\\n"
        "\\midrule\n"
        "\\endhead\n"
        "\\midrule\n"
        "\\multicolumn{4}{r}{Continued on next page} \\\\\n"
        "\\midrule\n"
        "\\endfoot\n"
        "\\bottomrule\n"
        "\\endlastfoot\n"
        "(6.40, 7.40) & (0.05, 0.15) & 4.000 & 0.300 \\\\\n"
        " & (0.30, 0.40) & 5.000 & 0.400 \\\\\n"
        "\\end{longtable}\n"
    )
    assert out.read_text() == expected
    assert result == expected
