import pytest

from gxana import config
from gxana.stages import runner as rn
from gxana.stages.runner import Command

KNOWN = ("b", "c", "a")


def _no_runner(*a, **k):
    raise AssertionError("dry run must not call the runner")


class Rc:
    def __init__(self, returncode):
        self.returncode = returncode


def _recorder(codes=None):
    calls = []

    def run(argv, **kwargs):
        calls.append((list(argv), kwargs))
        return Rc((codes or {}).get(len(calls), 0))
    return calls, run


def test_command_defaults_cwd_to_none():
    assert Command(["x"], "s") == (["x"], "s", None)


def test_check_steps_accepts_known_steps():
    rn.check_steps(["a", "b"], KNOWN)
    rn.check_steps([], KNOWN, first="sorted")


def test_check_steps_given_names_the_first_unknown_in_given_order():
    with pytest.raises(config.ConfigError, match=r"^unknown step 'zzz'; known: \['b', 'c', 'a'\]$"):
        rn.check_steps(["a", "zzz", "plot"], KNOWN)


def test_check_steps_sorted_names_the_alphabetically_first_unknown():
    with pytest.raises(config.ConfigError, match=r"^unknown step 'plot'; known: \['b', 'c', 'a'\]$"):
        rn.check_steps(["a", "zzz", "plot"], KNOWN, first="sorted")


def test_check_steps_moved_is_checked_step_by_step_before_unknown():
    moved = {"tables": "barlow"}
    with pytest.raises(config.ConfigError, match=r"^step 'tables' moved to `gxana run barlow`$"):
        rn.check_steps(["a", "tables", "zzz"], KNOWN, moved=moved)
    with pytest.raises(config.ConfigError, match=r"^unknown step 'zzz'"):
        rn.check_steps(["zzz", "tables"], KNOWN, moved=moved)


def test_check_steps_rejects_a_bad_first():
    with pytest.raises(ValueError, match="first"):
        rn.check_steps(["a"], KNOWN, first="alphabetical")


def test_command_line_quotes_and_wraps_cwd():
    assert rn.command_line(Command(["echo", "a b"], "s")) == "echo 'a b'"
    assert rn.command_line(Command(["ls"], "s", "/w d")) == "(cd '/w d' && ls)"


def test_run_commands_dry_run_prints_and_runs_nothing(capsys):
    calls, run = _recorder()
    assert rn.run_commands([Command(["a"], "s"), Command(["b"], "s", "/w")], True, run) == 0
    assert calls == []
    assert capsys.readouterr().out == "a\n(cd /w && b)\n"


def test_run_commands_passes_cwd_only_when_set():
    calls, run = _recorder()
    assert rn.run_commands([Command(["a"], "s"), Command(["b"], "s", "/w")], False, run) == 0
    assert calls == [(["a"], {"check": False}), (["b"], {"check": False, "cwd": "/w"})]


def test_run_commands_stops_at_the_first_failure(capsys):
    calls, run = _recorder({2: 5})
    cmds = [Command([x], "s") for x in ("a", "b", "c")]
    assert rn.run_commands(cmds, False, run) == 5
    assert [c[0] for c in calls] == [["a"], ["b"]]
    assert capsys.readouterr().out == "a\nb\n"


def test_run_commands_result_without_returncode_counts_as_success():
    assert rn.run_commands([Command(["a"], "s")], False, lambda *a, **k: object()) == 0
    assert rn.run_commands([Command(["a"], "s")], False, lambda *a, **k: Rc(None)) == 0


def test_run_steps_walks_known_order_and_plans_each_step_after_before(capsys):
    events = []

    def plan(step):
        events.append(("plan", step))
        return [Command([step], step)]

    calls, run = _recorder()
    rc = rn.run_steps(["a", "b"], KNOWN, plan, dry_run=False, runner=run,
                      before=lambda s: events.append(("before", s)), after=lambda s: events.append(("after", s)))
    assert rc == 0
    assert events == [("before", "b"), ("plan", "b"), ("after", "b"), ("before", "a"), ("plan", "a"), ("after", "a")]
    assert [c[0] for c in calls] == [["b"], ["a"]]


def test_run_steps_runs_a_repeated_step_once():
    calls, run = _recorder()
    assert rn.run_steps(["a", "a"], KNOWN, lambda s: [Command([s], s)], dry_run=False, runner=run) == 0
    assert calls == [(["a"], {"check": False})]


def test_run_steps_before_return_value_ends_the_run():
    planned = []
    rc = rn.run_steps(["b", "a"], KNOWN, lambda s: planned.append(s) or [], dry_run=False, runner=_no_runner,
                      before=lambda s: 7 if s == "a" else None)
    assert rc == 7 and planned == ["b"]


def test_run_steps_failure_skips_after_and_later_steps():
    after = []
    calls, run = _recorder({1: 2})
    rc = rn.run_steps(["b", "a"], KNOWN, lambda s: [Command([s], s)], dry_run=False, runner=run,
                      after=after.append)
    assert rc == 2 and after == [] and len(calls) == 1


def test_run_steps_dry_run_still_calls_before_and_after(capsys):
    seen = []
    rc = rn.run_steps(["c"], KNOWN, lambda s: [Command([s], s)], dry_run=True, runner=_no_runner,
                      before=lambda s: seen.append("before"), after=lambda s: seen.append("after"))
    assert rc == 0 and seen == ["before", "after"]
    assert capsys.readouterr().out == "c\n"


def test_executable_prefers_the_built_binary(tmp_path):
    (tmp_path / "build" / "bin").mkdir(parents=True)
    (tmp_path / "build" / "bin" / "tool").write_text("")
    env = {"GXANA_ROOT": str(tmp_path)}
    assert rn.executable("tool", env) == str(tmp_path / "build" / "bin" / "tool")
    assert rn.executable("other", env) == "other"


def test_python_module_and_num():
    import sys
    assert rn.python_module("gxana_xsection", "weighted_average", "a", "--pattern", "p") == [
        sys.executable, "-m", "gxana_xsection.weighted_average", "a", "--pattern", "p"]
    assert [rn.num(v) for v in (6.4, 2, 0.051, 11.40, "x")] == ["6.4", "2", "0.051", "11.4", "x"]


def test_root_macro_without_args_calls_the_macro_defaults():
    env = {"GXANA_ROOT": "/r"}
    assert rn.root_macro(env, "ch", "m/A.C") == ["root", "-l", "-b", "-q", "/r/rootlogon.C", "/r/analyses/ch/m/A.C"]


def test_root_macro_writes_args_as_cxx_literals():
    argv = rn.root_macro({"GXANA_ROOT": "/r"}, "ch", "A.C", ["x y", 4, 0.5, True, False])
    assert argv[-1] == '/r/analyses/ch/A.C("x y",4,0.5,true,false)'


def test_root_macro_empty_args_keep_the_parentheses():
    assert rn.root_macro({"GXANA_ROOT": "/r"}, "ch", "A.C", [])[-1] == "/r/analyses/ch/A.C()"


@pytest.mark.parametrize("bad", ['a"b', "a\\b", None, [1], {"a": 1}])
def test_cxx_arg_rejects(bad):
    with pytest.raises(config.ConfigError, match="macro argument"):
        rn.cxx_arg(bad)
