#!/usr/bin/env python3
"""Rewrite legacy absolute FSU paths in migrated copies (spec §7.3).

  uv run python scripts/migrate_paths.py analyses/kpkpxim/selection/*.C

C/C++: "…/legacy/rest" -> gxana::EnvPath("VAR", "rest") (+ .c_str() when the
literal is not concatenated; `const char* x = "…"` becomes std::string).
Python: os.path.join(os.environ["VAR"], "rest").  Shell: "${VAR}/rest".
Never touches _workdir/. Exit 1 if any literal has no mapping.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path
from typing import List, Sequence, Tuple

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "packages" / "common" / "python"))
from gxana.paths import legacy_to_env  # noqa: E402

LEGACY = re.compile(r'"(/d/grid17/hjesse/[^"]*)"')
TEMPLATE = re.compile(r"^\$\{(GXANA_[A-Z]+)\}/?(.*)$", re.S)
INCLUDE = '#include "gxana/common/Paths.h"\n'
KINDS = {".C": "cpp", ".cpp": "cpp", ".cxx": "cpp", ".h": "cpp", ".py": "py", ".sh": "sh"}


def _split(literal: str) -> Tuple[str, str]:
    m = TEMPLATE.match(legacy_to_env(literal))
    return (m.group(1), m.group(2)) if m else ("", "")


def _cpp_line(line: str, unmapped: List[str]) -> str:
    decl = re.match(r'^(\s*)const\s+char\s*\*\s*(\w+)\s*=\s*"(/d/grid17/hjesse/[^"]*)"\s*;', line)
    if decl:
        var, rest = _split(decl.group(3))
        if var:
            return f'{decl.group(1)}std::string {decl.group(2)} = gxana::EnvPath("{var}", "{rest}");' + line[decl.end():]

    def sub(m: "re.Match[str]") -> str:
        var, rest = _split(m.group(1))
        if not var:
            unmapped.append(m.group(1))
            return m.group(0)
        before = line[:m.start()].rstrip()
        after = line[m.end():].lstrip()
        call = f'gxana::EnvPath("{var}", "{rest}")'
        concatenated = before.endswith("+") or after.startswith("+") or re.search(r"=\s*$", before) is not None
        return call if concatenated else call + ".c_str()"

    return LEGACY.sub(sub, line)


def _add_include(text: str) -> str:
    if INCLUDE in text:
        return text
    return INCLUDE + text


def rewrite(text: str, kind: str) -> Tuple[str, List[str]]:
    unmapped: List[str] = []
    if kind == "cpp":
        out = "".join(_cpp_line(l, unmapped) for l in text.splitlines(keepends=True))
        if out != text:
            out = _add_include(out)
        return out, unmapped

    def sub(m: "re.Match[str]") -> str:
        var, rest = _split(m.group(1))
        if not var:
            unmapped.append(m.group(1))
            return m.group(0)
        if kind == "py":
            return f'os.path.join(os.environ["{var}"], "{rest}")'
        return f'"${{{var}}}/{rest}"'

    out = LEGACY.sub(sub, text)
    if kind == "py" and out != text and not re.search(r"^import os\b", out, re.M):
        if out.startswith("#!"):
            first, _, rest = out.partition("\n")
            out = first + "\nimport os\n" + rest
        else:
            out = "import os\n" + out
    return out, unmapped


def main(argv: Sequence[str]) -> int:
    bad = 0
    for arg in argv:
        path = Path(arg).resolve()
        if (ROOT / "_workdir") in path.parents:
            sys.exit(f"refusing to edit legacy source {path}")
        kind = KINDS.get(path.suffix)
        if kind is None:
            continue
        text = path.read_text()
        out, unmapped = rewrite(text, kind)
        for lit in unmapped:
            n = next(i for i, l in enumerate(text.splitlines(), 1) if lit in l)
            print(f"{arg}:{n}: unmapped {lit}")
            bad += 1
        if out != text:
            path.write_text(out)
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
