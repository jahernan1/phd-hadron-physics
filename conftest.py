"""Refuse to test this checkout with another checkout's gxana.

A shell that sourced another checkout's env/setup.sh exports that checkout's
GXANA_ROOT and puts its packages first on PYTHONPATH; the tests here would
then import and run the other checkout's code (docs/environment.md)."""
from pathlib import Path

import pytest

from gxana.paths import foreign_checkout


def pytest_sessionstart(session):
    problems = foreign_checkout(Path(__file__).resolve().parent)
    if problems:
        pytest.exit("this shell points at another gxana checkout (" + "; ".join(problems) + "). "
                    "Source this checkout's env/setup.sh, or run in a clean environment.", returncode=4)
