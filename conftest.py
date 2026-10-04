"""Refuse to test this checkout with another checkout's gxana.

A shell that sourced another checkout's env/setup.sh exports that checkout's
GXANA_ROOT and puts its packages first on PYTHONPATH; the tests here would
then import and run the other checkout's code (docs/environment.md)."""
import os
from pathlib import Path

import pytest


def pytest_sessionstart(session):
    checkout = Path(__file__).resolve().parent
    try:
        from gxana.paths import foreign_checkout
    except ImportError:  # gxana not importable here: check GXANA_ROOT alone
        value = os.environ.get("GXANA_ROOT")
        problems = [f"GXANA_ROOT={value}"] if value and Path(value).resolve() != checkout else []
    else:
        problems = foreign_checkout(checkout)
    if problems:
        pytest.exit("this shell points at another gxana checkout (" + "; ".join(problems) + "). "
                    "Source this checkout's env/setup.sh, or run in a clean environment.", returncode=4)
