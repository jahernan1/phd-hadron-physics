#!/usr/bin/env python3
"""Freeze a legacy fit macro (or some of its lines) as a traced copy.

usage: freeze.py SRC OUT ORIGIN [A:B ...]
  SRC     the macro text to copy (a git show of the commit before the port)
  OUT     the frozen copy to write
  ORIGIN  the line naming the origin, written into the header (e.g. "analyses/... at 9a61032")
  A:B     1-based inclusive line ranges of SRC to keep (default: the whole file)

The only changes to the copied lines: "w->factory(" becomes "LegacyFactory(w, ", and
"LegacyFitDone(w, data);" is inserted after every statement containing "->fitTo(" (see
LegacyTrace.h). The header records both.
"""
import sys


def code(line):
    return line.split("//", 1)[0].rstrip()


def main(argv):
    src, out, origin = argv[1], argv[2], argv[3]
    lines = open(src).read().split("\n")
    if lines and lines[-1] == "":
        lines.pop()
    if len(argv) > 4:
        kept = []
        for r in argv[4:]:
            a, b = (int(x) for x in r.split(":"))
            kept += lines[a - 1 : b]
    else:
        kept = lines
    body, i, n_factory, n_fit = [], 0, 0, 0
    while i < len(kept):
        line = kept[i]
        if "w->factory(" in line:
            n_factory += line.count("w->factory(")
            line = line.replace("w->factory(", "LegacyFactory(w, ")
        body.append(line)
        if "->fitTo(" in code(line):
            while not code(kept[i]).endswith(");"):
                i += 1
                body.append(kept[i])
            indent = line[: len(line) - len(line.lstrip())]
            body.append(indent + "LegacyFitDone(w, data);")
            n_fit += 1
        i += 1
    header = [
        "// FROZEN LEGACY COPY for the gxana::fit equivalence checks. Do not edit.",
        "// Origin: " + origin + (" lines " + " ".join(argv[4:]) if len(argv) > 4 else " (whole file)"),
        "// Made by packages/fit/tests/legacy_sites/freeze.py: \"w->factory(\" -> \"LegacyFactory(w, \" (%d, comments included),"
        % n_factory,
        "// \"LegacyFitDone(w, data);\" after each fitTo statement (%d). Nothing else changed." % n_fit,
        '#include "LegacyTrace.h"',
    ]
    open(out, "w").write("\n".join(header + body) + "\n")
    print("froze %s -> %s: %d factory, %d fitTo" % (src, out, n_factory, n_fit))


if __name__ == "__main__":
    main(sys.argv)
