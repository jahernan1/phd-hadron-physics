#!/usr/bin/env bash
# Fetch (if needed) and build the patched halld_sim for one sim version set.
# Usage: build_halld_sim.sh [--dry-run] [-j N] <version-set>   e.g. recon-2018_08-ver02_31
# Needs the GlueX container with /group/halld (docs/environment.md). The
# checkout lives at $GXANA_EXTERNALS/halld_sim-<set>, the home= that
# env/version_sets/<set>.xml.in gives halld_sim.
# No `set -u`: the JLab environment scripts sourced below read unset variables.
set -eo pipefail
dry=0; jobs=8
while [ $# -gt 0 ]; do
    case "$1" in
        --dry-run) dry=1; shift ;;
        -j) jobs=$2; shift 2 ;;
        -*) echo "build_halld_sim.sh: unknown option $1" >&2; exit 2 ;;
        *) break ;;
    esac
done
if [ $# -ne 1 ]; then echo "usage: build_halld_sim.sh [--dry-run] [-j N] <version-set>" >&2; exit 2; fi
set_name=$1
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
if [ ! -f "$root/env/version_sets/$set_name.xml.in" ]; then
    echo "build_halld_sim.sh: unknown version set $set_name (see env/version_sets/)" >&2; exit 2
fi
dest="${GXANA_EXTERNALS:-$root/_externals}/halld_sim-$set_name"
if [ "$dry" = 1 ]; then
    echo "+ source env/setup.sh --sim=$set_name"
    echo "+ gxana externals fetch halld_sim --dest $dest"
    echo "+ cd $dest/src && scons -u -j$jobs install"
    exit 0
fi
. "$root/env/setup.sh" "--sim=$set_name"
gxana externals fetch halld_sim --dest "$dest"
if [ "${HALLD_SIM_HOME:-}" != "$dest" ]; then
    echo "build_halld_sim.sh: HALLD_SIM_HOME=${HALLD_SIM_HOME:-unset}, expected $dest (template/home mismatch)" >&2; exit 1
fi
cd "$dest/src" && scons -u "-j$jobs" install
