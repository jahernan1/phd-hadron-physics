"""The fork engine compiles with its own example configs (packages/qfactors).

Syntax-only g++ checks against ROOT's headers: main.C with the fork's
configPDFs.h and with each pdfTemplates example swapped in."""
import shutil
import subprocess
from pathlib import Path

import pytest

ENGINE = Path(__file__).resolve().parents[2] / "packages" / "qfactors"
pytestmark = [
    pytest.mark.skipif(not (ENGINE / "main.C").is_file(), reason="packages/qfactors not checked out"),
    pytest.mark.skipif(shutil.which("root-config") is None, reason="ROOT not on PATH"),
]


def _syntax_check(workdir):
    flags = subprocess.run(["root-config", "--cflags"], capture_output=True, text=True, check=True).stdout.split()
    return subprocess.run(["g++", "-fsyntax-only", "main.C", *flags], cwd=workdir,
                          capture_output=True, text=True, timeout=600)


@pytest.mark.parametrize("config", [None, "auxilliary/pdfTemplates/configPDFs_sample1D.h",
                                    "auxilliary/pdfTemplates/configPDFs_sample2D.h"])
def test_engine_compiles_with_example_config(config, tmp_path):
    work = tmp_path / "engine"
    shutil.copytree(ENGINE, work, ignore=shutil.ignore_patterns(".git"))
    if config:
        shutil.copy2(work / config, work / "configPDFs.h")
    proc = _syntax_check(work)
    assert proc.returncode == 0, (proc.stdout + proc.stderr)[-4000:]


def test_main_has_no_variable_length_param_array():
    text = (ENGINE / "main.C").read_text()
    assert "std::vector<float> vecParams" in text
    assert "float vecParams[" not in text


def test_merge_carries_chisqndf():
    text = (ENGINE / "mergeQresults.C").read_text()
    assert 'SetBranchAddress(("chiSqNdf_"+s_discrimVar2)' in text
    assert 'Branch(("chiSqNdf_"+s_discrimVar2)' in text
