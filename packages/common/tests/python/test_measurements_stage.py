"""gxana run measurements: the commands planned from analyses/<channel>/config/measurements.yaml."""
import copy

import pytest

from gxana import cli
from gxana import config as gconfig
from gxana.stages import measurements as ms

ENV = {"GXANA_ROOT": "/r", "GXANA_DATA": "/d", "GXANA_OUTPUT": "/o"}
HEAD = ["root", "-l", "-b", "-q", "/r/rootlogon.C"]
M = "/r/analyses/kpkpxim/measurements"


class Rc:
    def __init__(self, returncode):
        self.returncode = returncode


def _cfg(channel="kpkpxim"):
    return copy.deepcopy(gconfig.load_channel(channel))


def _recorder(codes=None):
    calls = []

    def run(argv, **kwargs):
        calls.append((argv[-1], kwargs))
        return Rc((codes or {}).get(len(calls), 0))
    return calls, run


def test_kpkpxim_plan_runs_each_macro_from_the_output_dir():
    cmds = ms.plan(_cfg(), list(ms.STEPS), environ=ENV)
    assert [(c.step, c.argv[-1]) for c in cmds] == [
        ("prep", f"{M}/mass/PrepMass.C(0)"), ("prep", f"{M}/lifetime/PrepLifetime.C(0)"),
        ("prep", f"{M}/spin/PrepSpinData.C(0)"), ("fit", f"{M}/mass/FitMass.C(0)"),
        ("fit", f"{M}/lifetime/FitLifetime.C(0)"), ("fit", f"{M}/spin/PlotGlueXSpin.C(0)")]
    assert all(c.argv[:5] == HEAD for c in cmds)
    assert {c.cwd for c in cmds} == {"/o/kpkpxim/measurements"}


def test_kpkpxim_measurements_single_threaded():
    cmds = ms.plan(_cfg(), list(ms.STEPS), environ=ENV)
    assert len(cmds) == 6 and all(c.argv[-1].endswith(".C(0)") for c in cmds)
    kml = ms.plan(_cfg("kpkpkmlamb"), list(ms.STEPS), environ=ENV)
    assert [c.argv[-1].rsplit("/", 1)[1] for c in kml] == ["FitXimStar.C(4,false)", "FitXimStar.C(4,true)"]


def test_kpkpkmlamb_plan_needs_no_kpkpxim_literal():
    cfg = _cfg("kpkpkmlamb")
    cmds = ms.plan(cfg, list(ms.STEPS), environ=ENV)
    assert [c.argv[-1] for c in cmds] == ["/r/analyses/kpkpkmlamb/measurements/FitXimStar.C(4,false)",
                                          "/r/analyses/kpkpkmlamb/measurements/FitXimStar.C(4,true)"]
    assert {c.cwd for c in cmds} == {"/o/kpkpkmlamb/measurements"}
    assert "kpkpxim" not in repr(cmds)


def test_items_and_steps_select():
    cmds = ms.plan(_cfg(), ["fit"], items=["spin"], environ=ENV)
    assert [c.argv[-1] for c in cmds] == [f"{M}/spin/PlotGlueXSpin.C(0)"]


def test_args_become_the_call():
    cfg = _cfg()
    cfg["measurements"]["items"]["mass"]["prep"]["args"] = [0]
    assert ms.plan(cfg, ["prep"], items=["mass"], environ=ENV)[0].argv[-1] == f"{M}/mass/PrepMass.C(0)"


def test_unknown_item_and_step_are_errors():
    with pytest.raises(gconfig.ConfigError, match="unknown item 'nope'"):
        ms.plan(_cfg(), ["fit"], items=["nope"], environ=ENV)
    with pytest.raises(gconfig.ConfigError, match="unknown step 'plot'"):
        ms.plan(_cfg(), ["plot"], environ=ENV)


def _set(path, value):
    def edit(cfg):
        node = cfg["measurements"]
        for key in path[:-1]:
            node = node[key]
        if value is KeyError:
            del node[path[-1]]
        else:
            node[path[-1]] = value
    return edit


@pytest.mark.parametrize("edit, match", [
    (_set(["output_dir"], KeyError), r"measurements\.output_dir: required"),
    (_set(["make_dirs"], "x"), r"measurements\.make_dirs: need a list"),
    (_set(["items"], {}), r"measurements\.items: need a mapping"),
    (_set(["items", "mass"], {}), r"measurements\.items\.mass: needs prep and/or fit"),
    (_set(["items", "mass", "prep", "threads"], 4), r"measurements\.items\.mass\.prep\.threads: unknown key"),
    (_set(["items", "mass", "prep", "macro"], ""), r"measurements\.items\.mass\.prep\.macro: need a non-empty string"),
    (_set(["items", "mass", "prep", "args"], 4), r"measurements\.items\.mass\.prep\.args: need a list"),
    (_set(["items", "mass", "prep", "args"], ['a"b']), "macro argument"),
])
def test_config_errors(edit, match):
    cfg = _cfg()
    edit(cfg)
    with pytest.raises(gconfig.ConfigError, match=match):
        ms.plan(cfg, ["prep"], environ=ENV)


def test_run_creates_the_directories_first(monkeypatch, tmp_path):
    monkeypatch.setattr(ms, "check_macros", lambda *a, **k: None)
    env = dict(ENV, GXANA_OUTPUT=str(tmp_path / "o"))
    calls, run = _recorder()
    assert ms.run_measurements(_cfg(), ["prep", "fit"], runner=run, environ=env, items=["mass"]) == 0
    work = str(tmp_path / "o" / "kpkpxim" / "measurements")
    assert (tmp_path / "o" / "kpkpxim" / "prod_plots").is_dir()
    assert calls == [(f"{M}/mass/PrepMass.C(0)", {"check": False, "cwd": work}),
                     (f"{M}/mass/FitMass.C(0)", {"check": False, "cwd": work})]


def test_run_stops_at_the_first_failure(monkeypatch, tmp_path):
    monkeypatch.setattr(ms, "check_macros", lambda *a, **k: None)
    env = dict(ENV, GXANA_OUTPUT=str(tmp_path / "o"))
    calls, run = _recorder({1: 6})
    assert ms.run_measurements(_cfg(), ["prep", "fit"], runner=run, environ=env) == 6
    assert len(calls) == 1


def test_dry_run_creates_nothing(monkeypatch, tmp_path, capsys):
    monkeypatch.setattr(ms, "check_macros", lambda *a, **k: None)
    env = dict(ENV, GXANA_OUTPUT=str(tmp_path / "o"))
    calls, run = _recorder()
    assert ms.run_measurements(_cfg(), ["prep"], dry_run=True, runner=run, environ=env, items=["mass"]) == 0
    assert calls == [] and not (tmp_path / "o").exists()
    out = capsys.readouterr().out
    assert out == f"(cd {tmp_path}/o/kpkpxim/measurements && root -l -b -q /r/rootlogon.C '{M}/mass/PrepMass.C(0)')\n"


def test_missing_macro_is_a_config_error(tmp_path):
    root = tmp_path / "r"
    (root / "analyses" / "kpkpkmlamb" / "measurements").mkdir(parents=True)
    env = dict(ENV, GXANA_ROOT=str(root), GXANA_OUTPUT=str(tmp_path / "o"))
    calls, run = _recorder()
    with pytest.raises(gconfig.ConfigError, match=r"measurements\.items\.ximstar\.fit: macro .*FitXimStar\.C does not exist"):
        ms.run_measurements(_cfg("kpkpkmlamb"), ["fit"], runner=run, environ=env)
    assert calls == [] and not (tmp_path / "o").exists()
    (root / "analyses" / "kpkpkmlamb" / "measurements" / "FitXimStar.C").write_text("")
    assert ms.run_measurements(_cfg("kpkpkmlamb"), ["fit"], runner=run, environ=env) == 0


def test_no_selected_item_has_the_step():
    with pytest.raises(gconfig.ConfigError, match=r"no selected item has step\(s\) prep"):
        ms.run_measurements(_cfg("kpkpkmlamb"), ["prep"], dry_run=True, environ=ENV)


def test_cli_dry_run(monkeypatch, tmp_path, capsys):
    monkeypatch.setenv("GXANA_OUTPUT", str(tmp_path / "o"))
    assert cli.main(["run", "measurements", "--channel", "kpkpkmlamb", "--item", "ximstar_tcut", "--dry-run"]) == 0
    assert capsys.readouterr().out.rstrip().endswith("FitXimStar.C(4,true)')")
    assert cli.main(["run", "measurements", "--channel", "kpkpkmlamb", "--steps", "prep", "--dry-run"]) == 2
