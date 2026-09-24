import importlib.util
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("migrate_paths", ROOT / "scripts" / "migrate_paths.py")
mp = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mp)


def test_cpp_concatenation():
    src = 'auto f = TFile::Open(("/d/grid17/hjesse/AnalysisNote/flatTrees/"+name+".root").c_str());\n'
    out, unmapped = mp.rewrite(src, "cpp")
    assert '(gxana::EnvPath("GXANA_DATA", "flatTrees/")+name+".root")' in out
    assert out.startswith('#include "gxana/common/Paths.h"\n')
    assert unmapped == []


def test_cpp_whole_literal_argument_gets_c_str():
    out, _ = mp.rewrite('TFile::Open("/d/grid17/hjesse/AnalysisNote/fluxFiles/f.root");\n', "cpp")
    assert 'TFile::Open(gxana::EnvPath("GXANA_DATA", "flux/f.root").c_str());' in out


def test_cpp_const_char_pointer_becomes_std_string():
    out, _ = mp.rewrite('const char* dir = "/d/grid17/hjesse/AnalysisNote/xsection/plots/";\n', "cpp")
    assert 'std::string dir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/xsection/plots/");' in out


def test_cpp_sprintf_format_keeps_placeholder():
    out, _ = mp.rewrite('sprintf(p, "/d/grid17/hjesse/AnalysisNote/xsection/plots/%s.pdf", n);\n', "cpp")
    assert 'sprintf(p, gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/xsection/plots/%s.pdf").c_str(), n);' in out


def test_include_added_once_and_after_existing_header_comment():
    src = '// header\n#include <TFile.h>\nA("/d/grid17/hjesse/Trees/x"); B("/d/grid17/hjesse/Trees/y");\n'
    out, _ = mp.rewrite(src, "cpp")
    assert out.count('#include "gxana/common/Paths.h"') == 1


def test_python_and_shell():
    out, _ = mp.rewrite('d = "/d/grid17/hjesse/AnalysisNote/xsection/data/"\n', "py")
    assert 'd = os.path.join(os.environ["GXANA_OUTPUT"], "kpkpxim/xsection/data/")' in out
    assert "import os" in out
    out, _ = mp.rewrite('root -q "/d/grid17/hjesse/AnalysisNote/utilities/flatTreePrep.C"\n', "sh")
    assert '"${GXANA_OUTPUT}/kpkpxim/utilities/flatTreePrep.C"' in out


def test_unmapped_reported():
    _, unmapped = mp.rewrite('x("/d/grid17/hjesse/random/thing");\n', "cpp")
    assert unmapped == ["/d/grid17/hjesse/random/thing"]


def test_refuses_workdir(tmp_path):
    with pytest.raises(SystemExit):
        mp.main([str(ROOT / "_workdir" / "anything.C")])


def test_python_shebang_stays_first():
    out, _ = mp.rewrite('#!/usr/bin/env python3\nd = "/d/grid17/hjesse/Trees/x"\n', "py")
    assert out.startswith("#!/usr/bin/env python3\nimport os\n")
