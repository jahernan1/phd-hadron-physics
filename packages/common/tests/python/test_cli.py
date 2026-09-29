import sys

import pytest

from gxana import cli


@pytest.fixture
def gxana_env(monkeypatch, tmp_path):
    for var in ("GXANA_DATA", "GXANA_OUTPUT", "GXANA_SCRATCH"):
        monkeypatch.setenv(var, str(tmp_path / var.lower()))
    return tmp_path


def test_help_lists_commands(capsys):
    with pytest.raises(SystemExit) as exc:
        cli.main(["--help"])
    assert exc.value.code == 0
    out = capsys.readouterr().out
    assert "doctor" in out
    assert "config" in out
    assert "run" in out


def test_config_show(capsys):
    assert cli.main(["config", "show", "--channel", "kpkpxim"]) == 0
    out = capsys.readouterr().out
    assert "reaction: kpkpxim" in out
    assert "2018-08" in out


def test_select_dry_run(gxana_env, capsys):
    rc = cli.main(["run", "select", "--channel", "kpkpxim", "--period", "2018-08", "--dry-run"])
    out = capsys.readouterr().out
    assert rc == 0
    assert "kpkpxim__B4_M23_2018-08_ana02" in out
    assert "DPROOFLiteManager::Process_Chain" in out


def test_xsection_dry_run(gxana_env, capsys):
    rc = cli.main(["run", "xsection", "--channel", "kpkpxim", "--steps", "tables", "--dry-run"])
    out = capsys.readouterr().out
    assert rc == 0
    assert "gxana_xsec_tables" in out
    assert "--fit Johnson" in out


def test_xsection_default_steps_omit_qvalue(gxana_env, capsys):
    from gxana.stages import xsection

    rc = cli.main(["run", "xsection", "--channel", "kpkpxim", "--dry-run"])
    out = capsys.readouterr().out
    assert rc == 0
    for step in xsection.DEFAULT_STEPS:
        assert step != "qvalue"
    assert "qvalue_rescale" not in out


def test_select_without_env_is_clean_error(monkeypatch, capsys):
    monkeypatch.delenv("GXANA_DATA", raising=False)
    rc = cli.main(["run", "select", "--channel", "kpkpxim", "--period", "2018-08", "--dry-run"])
    assert rc == 2
    assert "gxana: error: GXANA_DATA is not set" in capsys.readouterr().err


def test_doctor_works_without_pyyaml(gxana_env, monkeypatch, capsys):
    monkeypatch.setitem(sys.modules, "yaml", None)
    rc = cli.main(["doctor"])
    out = capsys.readouterr().out
    assert rc == 1
    assert "[fail] pyyaml" in out


def test_unknown_channel_is_clean_error(capsys):
    assert cli.main(["config", "show", "--channel", "nope"]) == 2
    assert "no config files" in capsys.readouterr().err


def test_select_error_is_clean_no_traceback(gxana_env, capsys):
    rc = cli.main([
        "run", "select", "--channel", "kpkpxim", "--period", "2018-08",
        "--thrown", "--sample", "F1", "--dry-run",
    ])
    err = capsys.readouterr().err
    assert rc == 2
    assert err.startswith("gxana: error: ")
    assert "Traceback" not in err


def test_barlow_dry_run(gxana_env, capsys):
    assert cli.main(["run", "barlow", "--channel", "kpkpxim", "--dry-run"]) == 0
    out = capsys.readouterr().out
    assert "gxana_barlow_trees" in out and "gxana_barlow_plot" in out
    assert "gxana_barlow_trees --check" not in out  # check is opt-in


def test_barlow_unknown_step_is_clean_error(gxana_env, capsys):
    assert cli.main(["run", "barlow", "--channel", "kpkpxim", "--steps", "barlow", "--dry-run"]) == 2
    assert "unknown step 'barlow'" in capsys.readouterr().err
