import pytest

from gxana import cli, config
from gxana.stages import systematics as sy


def test_no_steps_yet():
    assert sy.STEPS == () and sy.DEFAULT_STEPS == ()


@pytest.mark.parametrize("step", ["bin", "tables", "weight", "barlow"])
def test_moved_steps_point_to_run_barlow(step):
    with pytest.raises(config.ConfigError, match=r"moved to `gxana run barlow`"):
        sy.run_systematics({}, [step])


def test_unknown_step_rejected():
    with pytest.raises(config.ConfigError, match="unknown step 'plot'"):
        sy.run_systematics({}, ["plot"])


def test_cli_rejects_moved_steps(capsys, monkeypatch, tmp_path):
    for var in ("GXANA_DATA", "GXANA_OUTPUT", "GXANA_SCRATCH"):
        monkeypatch.setenv(var, str(tmp_path / var.lower()))
    assert cli.main(["run", "systematics", "--channel", "kpkpxim", "--steps", "weight"]) == 2
    assert "gxana run barlow" in capsys.readouterr().err
