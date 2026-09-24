#!/usr/bin/env bash
# Copy code (not outputs) from a legacy dir into archive/. Usage: archive_copy.sh SRC DEST
set -euo pipefail
src=$1; dest=$2
case "$(cd "$(dirname "$dest")" && pwd)/$(basename "$dest")" in */_workdir/*) echo "refusing: dest in _workdir" >&2; exit 1;; esac
mkdir -p "$dest"
(cd "$src" && find . -type f \( -name '*.C' -o -name '*.cpp' -o -name '*.cxx' -o -name '*.h' -o -name '*.py' -o -name '*.sh' -o -name '*.cfg' -o -name '*.conf' -o -name '*.config' -o -name '*.input' \) \
   -not -name '*~' -not -name '#*#' -not -name '.#*' -not -path '*/c_format_plots/*' -not -name '*_ACLiC_dict*' -size -1024k -print0) |
  while IFS= read -r -d '' f; do mkdir -p "$dest/$(dirname "$f")"; cp -p "$src/$f" "$dest/$f"; done
