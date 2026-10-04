# Source me from bash or zsh:  source env/setup.sh [--gluex | --sim=<set>]
# Exports GXANA_* (docs/REFACTOR_SPEC.md §7) plus library and python paths.
#   --gluex      boot the GlueX analysis environment (halld version set 5.12.0:
#                ROOT 6.24.04, gluex_root_analysis 1.25.0). Needs /group/halld.
#   --sim=<set>  boot the GlueX MC environment from env/version_sets/<set>.xml.in
#                (rendered to $GXANA_EXTERNALS/version_sets/<set>.xml; exports
#                GXANA_SIM_VERSION_SET). Exclusive with --gluex.
# GXANA_GLUEX_BOOT overrides the JLab boot script (used by the tests).
# Site values: pre-set variables or env/site.sh (see env/site.example.sh).
# GXANA_DATA defaults to $GXANA_ROOT/_data (gitignored); point it at the real
# Trees/ area via env/site.sh.
# NOTE: `source env/setup.sh` without arguments inside a script or function
# sees the caller's positional parameters ("$@") in bash and zsh, which then
# fail the "unknown option" check below (or, for a caller argument --gluex,
# boot GlueX). Scripts that source this file should run `set --` first.
# Sourcing it from a second checkout replaces the first checkout's
# PYTHONPATH/LD_LIBRARY_PATH/DYLD_LIBRARY_PATH entries and the defaults
# derived from its GXANA_ROOT; use one shell per checkout otherwise.

_gxana_cleanup() {
    unset -f _gxana_cleanup _gxana_prepend _gxana_strip
    unset _gxana_self _gxana_gluex _gxana_arg _gxana_boot _gxana_sim _gxana_tmpl _gxana_vs _gxana_old
}

# Idempotent prepend: _gxana_prepend VAR DIR adds DIR to the front of VAR
# only if DIR is not already one of VAR's colon-separated entries.
_gxana_prepend() {
    _gxana_var="$1"
    _gxana_dir="$2"
    eval "_gxana_val=\"\${$_gxana_var:-}\""
    case ":$_gxana_val:" in
        *":$_gxana_dir:"*) ;;
        *) _gxana_val="$_gxana_dir${_gxana_val:+:$_gxana_val}" ;;
    esac
    eval "export $_gxana_var=\"\$_gxana_val\""
    unset _gxana_var _gxana_dir _gxana_val
}

# _gxana_strip VAR DIR removes DIR and every DIR/... entry from the
# colon-separated VAR (unset if nothing is left); other entries keep their order.
_gxana_strip() {
    eval "_gxana_val=\"\${$1:-}\""
    if [ -n "$_gxana_val" ]; then
        _gxana_rest="$_gxana_val:"
        _gxana_new=""
        _gxana_n=0
        while [ -n "$_gxana_rest" ]; do
            _gxana_e="${_gxana_rest%%:*}"
            _gxana_rest="${_gxana_rest#*:}"
            case "$_gxana_e" in
                "$2"|"$2"/*) continue ;;
            esac
            if [ "$_gxana_n" = 0 ]; then _gxana_new="$_gxana_e"; else _gxana_new="$_gxana_new:$_gxana_e"; fi
            _gxana_n=1
        done
        if [ "$_gxana_n" = 0 ]; then eval "unset $1"; else eval "export $1=\"\$_gxana_new\""; fi
    fi
    unset _gxana_val _gxana_rest _gxana_new _gxana_n _gxana_e
}

if [ -n "${BASH_SOURCE:-}" ]; then
    _gxana_self="${BASH_SOURCE[0]}"
elif [ -n "${ZSH_VERSION:-}" ]; then
    _gxana_self="${(%):-%x}"
else
    echo "env/setup.sh: source this file from bash or zsh" >&2
    return 1
fi

_gxana_gluex=0
_gxana_sim=""
for _gxana_arg in "$@"; do
    case "$_gxana_arg" in
        --gluex) _gxana_gluex=1 ;;
        --sim=*)
            _gxana_sim="${_gxana_arg#--sim=}"
            case "$_gxana_sim" in
                ""|*/*)
                    echo "env/setup.sh: invalid sim version set '$_gxana_sim'" >&2
                    _gxana_cleanup
                    return 1
                    ;;
            esac
            ;;
        *) echo "env/setup.sh: unknown option $_gxana_arg (sourced from a script? it sees the script's arguments; run \`set --\` before sourcing)" >&2; _gxana_cleanup; return 1 ;;
    esac
done

_gxana_old="${GXANA_ROOT:-}"
GXANA_ROOT="$(cd "$(dirname "$_gxana_self")/.." && pwd)"
export GXANA_ROOT
if [ -n "$_gxana_old" ] && [ "$_gxana_old" != "$GXANA_ROOT" ]; then
    # Sourced before from another checkout: drop its paths and the defaults
    # derived from it, so this checkout's apply (user presets are kept).
    _gxana_strip PYTHONPATH "$_gxana_old"
    _gxana_strip LD_LIBRARY_PATH "$_gxana_old"
    _gxana_strip DYLD_LIBRARY_PATH "$_gxana_old"
    [ "${GXANA_DATA:-}" = "$_gxana_old/_data" ] && unset GXANA_DATA
    [ "${GXANA_OUTPUT:-}" = "$_gxana_old/_output" ] && unset GXANA_OUTPUT
    [ "${GXANA_EXTERNALS:-}" = "$_gxana_old/_externals" ] && unset GXANA_EXTERNALS
    [ "${GXANA_ANALYSIS_DATA:-}" = "$_gxana_old/gluex_analysis_data" ] && unset GXANA_ANALYSIS_DATA
fi
[ -f "$GXANA_ROOT/env/site.sh" ] && . "$GXANA_ROOT/env/site.sh"
export GXANA_DATA="${GXANA_DATA:-$GXANA_ROOT/_data}"
export GXANA_OUTPUT="${GXANA_OUTPUT:-$GXANA_ROOT/_output}"
export GXANA_SCRATCH="${GXANA_SCRATCH:-${TMPDIR:-/tmp}/gxana-${USER:-user}}"
export GXANA_EXTERNALS="${GXANA_EXTERNALS:-$GXANA_ROOT/_externals}"
export GXANA_ANALYSIS_DATA="${GXANA_ANALYSIS_DATA:-$GXANA_ROOT/gluex_analysis_data}"

if [ "$_gxana_gluex" = 1 ] && [ -n "$_gxana_sim" ]; then
    echo "env/setup.sh: --gluex and --sim are exclusive" >&2
    _gxana_cleanup
    return 1
fi

if [ -n "$_gxana_sim" ]; then
    _gxana_tmpl="$GXANA_ROOT/env/version_sets/$_gxana_sim.xml.in"
    if [ ! -f "$_gxana_tmpl" ]; then
        echo "env/setup.sh: unknown sim version set $_gxana_sim (see env/version_sets/)" >&2
        _gxana_cleanup
        return 1
    fi
    mkdir -p "$GXANA_EXTERNALS/version_sets"
    _gxana_vs="$GXANA_EXTERNALS/version_sets/$_gxana_sim.xml"
    sed "s|\${GXANA_EXTERNALS}|$GXANA_EXTERNALS|g" "$_gxana_tmpl" > "$_gxana_vs"
fi

if [ "$_gxana_gluex" = 1 ] || [ -n "$_gxana_sim" ]; then
    _gxana_boot="${GXANA_GLUEX_BOOT:-/group/halld/Software/build_scripts/gluex_env_boot_jlab.sh}"
    if [ ! -f "$_gxana_boot" ]; then
        echo "env/setup.sh: $_gxana_boot not found (bind /group or mount CVMFS)" >&2
        _gxana_cleanup
        return 1
    fi
    . "$_gxana_boot"
    if [ -n "$_gxana_sim" ]; then
        gxenv "$_gxana_vs"
        export GXANA_SIM_VERSION_SET="$_gxana_sim"
    else
        gxenv "${HALLD_VERSIONS:-/group/halld/www/halldweb/html/halld_versions}/version_5.12.0.xml"
        unset GXANA_SIM_VERSION_SET
    fi
fi

_gxana_prepend LD_LIBRARY_PATH "$GXANA_ROOT/build/lib"
_gxana_prepend DYLD_LIBRARY_PATH "$GXANA_ROOT/build/lib"
_gxana_prepend PYTHONPATH "$GXANA_ROOT/packages/barlow/python"
_gxana_prepend PYTHONPATH "$GXANA_ROOT/packages/studies/python"
_gxana_prepend PYTHONPATH "$GXANA_ROOT/packages/systematics/python"
_gxana_prepend PYTHONPATH "$GXANA_ROOT/packages/xsection/python"
_gxana_prepend PYTHONPATH "$GXANA_ROOT/packages/common/python"

# No `gxana` console script in the container (env/apptainer/gxana.def installs
# pyyaml/pytest only, not this package). Fall back to a shell function that
# invokes the module directly, unless a real `gxana` is already on PATH. This
# function intentionally persists after _gxana_cleanup runs.
if ! command -v gxana >/dev/null 2>&1; then
    gxana() { python3 -m gxana.cli "$@"; }
fi

_gxana_cleanup
