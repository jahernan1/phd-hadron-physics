"""Package code names no channel: the kpkpxim names reach the packages only through
analyses/kpkpxim/config. Scanned: the C++ sources, headers, apps and LinkDef.h and the
python modules of packages/{common,xsection,barlow,systematics,fit,studies}. Comments and docstrings may
name the legacy channel (they say where code came from); code and string literals may
not, except the counted entries of ALLOWED."""
import ast
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PACKAGES = ("common", "xsection", "barlow", "systematics", "fit", "studies")
TOKENS = ("kpkpxim", "decayxim", "hybrid_combo", "kphighrap", "#Xi", "#Lambda#pi")
# (file, token) -> occurrences kept on purpose.
ALLOWED = {
    # LEGACY_PREFIXES: migration-only rewrite table of the thesis-era absolute paths.
    ("packages/common/python/gxana/paths.py", "kpkpxim"): 7,
    # `gxana run <stage>` keeps --channel kpkpxim as its default (author decision).
    ("packages/common/python/gxana/cli.py", "kpkpxim"): 5,
}


def _cxx_code(text: str) -> str:
    """text without // and /* */ comments (string and character literals kept)."""
    out, i, n = [], 0, len(text)
    while i < n:
        c = text[i]
        if c in "\"'":
            j = i + 1
            while j < n and text[j] != c:
                j += 2 if text[j] == "\\" else 1
            out.append(text[i:j + 1])
            i = j + 1
        elif text.startswith("//", i):
            i = text.find("\n", i) if "\n" in text[i:] else n
        elif text.startswith("/*", i):
            i = text.find("*/", i) + 2
        else:
            out.append(c)
            i += 1
    return "".join(out)


def _python_strings(text: str) -> str:
    """Every string constant of the module except docstrings, one per line."""
    tree = ast.parse(text)
    docstrings = set()
    for node in ast.walk(tree):
        if isinstance(node, (ast.Module, ast.ClassDef, ast.FunctionDef, ast.AsyncFunctionDef)):
            body = getattr(node, "body", [])
            if body and isinstance(body[0], ast.Expr) and isinstance(body[0].value, ast.Constant):
                docstrings.add(id(body[0].value))
    return "\n".join(node.value for node in ast.walk(tree)
                     if isinstance(node, ast.Constant) and isinstance(node.value, str) and id(node) not in docstrings)


def _sources():
    for package in PACKAGES:
        base = ROOT / "packages" / package
        cxx = sorted(base.glob("*"))  # LinkDef.h
        cxx += [p for sub in ("src", "include", "apps") for p in sorted((base / sub).rglob("*"))]
        for path in cxx:
            if path.suffix in (".h", ".cxx", ".C", ".cpp"):
                yield path, _cxx_code(path.read_text())
        for path in sorted((base / "python").rglob("*.py")):
            yield path, _python_strings(path.read_text())


def test_scanner_sees_code_and_skips_comments():
    assert _cxx_code('a = "x // y"; // kpkpxim\n/* hybrid_combo */ b') == 'a = "x // y"; \n b'
    assert _python_strings('"""kpkpxim"""\nx = "decayxim_M"  # hybrid_combo\n') == "decayxim_M"


def test_no_channel_name_in_package_code():
    counts = {}
    for path, code in _sources():
        rel = path.relative_to(ROOT).as_posix()
        for token in TOKENS:
            if token in code:
                counts[(rel, token)] = code.count(token)
    assert counts == ALLOWED
