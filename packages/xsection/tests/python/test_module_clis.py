import subprocess
import sys

from gxana_xsection import components, tex_table, weighted_average


def _run(module, *args):
    return subprocess.run([sys.executable, "-m", f"gxana_xsection.{module}", *args],
                           cwd=None, capture_output=True, text=True, check=True)


def test_weighted_average_cli(tmp_path):
    src = tmp_path / "src"
    src.mkdir()
    for i, y in enumerate((2.0, 4.0, 3.0)):
        (src / f"diffxsec_P{i}_emin_6.40_emax_7.40.txt").write_text(
            f"tBinCenter\tdsigmadt\ttBinWidth\tYerr\n0.225  {y}  0.125  1.0\n0.44  {y}  0.09  1.0\n")

    api_out = tmp_path / "api_out"
    api_out.mkdir()
    api_result = weighted_average.weight_files(str(src), str(api_out), pattern="diffxsec*.txt")

    cli_out = tmp_path / "cli_out"
    cli_out.mkdir()
    _run("weighted_average", str(src), str(cli_out), "--pattern", "diffxsec*.txt")

    from pathlib import Path
    cli_result = cli_out / Path(api_result).name
    assert cli_result.read_text() == Path(api_result).read_text()


def test_components_cli(tmp_path):
    src = tmp_path / "src"
    src.mkdir()
    (src / "totout_flatTree_kpkpxim__2017-01.txt").write_text("e ee y ey\n6.9 0.5 1 2\n")

    api_out = tmp_path / "api_out"
    api_out.mkdir()
    components.split_files(str(src), str(api_out), pattern="totout*.txt", anchor="kpkpxim")

    cli_out = tmp_path / "cli_out"
    cli_out.mkdir()
    _run("components", str(src), str(cli_out), "--pattern", "totout*.txt", "--anchor", "kpkpxim")

    assert sorted(p.name for p in cli_out.iterdir()) == sorted(p.name for p in api_out.iterdir())
    for p in api_out.iterdir():
        assert (cli_out / p.name).read_text() == p.read_text()


def test_tex_table_cli(tmp_path):
    d = tmp_path / "data"
    d.mkdir()
    (d / "diffxsec_test_emin_6.40_emax_7.40.txt").write_text(
        "-t\tdsigmadt\tdelta_x\tdelta_y\tS\n"
        "0.10\t4.0\t0.05\t0.30\t0.80\n"
        "0.35\t5.0\t0.05\t0.40\t1.20\n"
    )

    api_out = tmp_path / "api.tex"
    tex_table.process_files_to_latex(str(d), "diffxsec*", "\t", str(api_out))

    cli_out = tmp_path / "cli.tex"
    _run("tex_table", str(d), "diffxsec*", str(cli_out), "--delimiter", "\t")

    assert cli_out.read_text() == api_out.read_text()
