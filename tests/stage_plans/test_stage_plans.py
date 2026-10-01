"""The stage plans, dry-run output and execution traces equal the recorded
baseline (tests/stage_plans/record.py; fixtures recorded from the code before
the stage-infrastructure port). On a failure, run
`uv run python tests/stage_plans/record.py` to see the diff."""
import importlib.util
from pathlib import Path

import pytest

_spec = importlib.util.spec_from_file_location("stage_plans_record", Path(__file__).with_name("record.py"))
record = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(record)


@pytest.fixture(scope="module")
def recorded():
    return record.record_all()


def test_baseline_holds_exactly_the_recorded_fixtures():
    assert sorted(p.name for p in record.BASELINE.iterdir()) == sorted(f"{n}.gz" for n in record.FIXTURE_NAMES)


@pytest.mark.parametrize("name", record.FIXTURE_NAMES)
def test_matches_baseline(recorded, name):
    assert recorded[name] == record.baseline_text(name), (
        f"{name} differs from tests/stage_plans/baseline; run `uv run python tests/stage_plans/record.py`")
