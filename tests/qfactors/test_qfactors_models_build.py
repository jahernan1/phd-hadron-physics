"""Every kpkpxim fit model compiles against the fork engine (syntax-only g++)."""
import shutil
from pathlib import Path

import pytest

from gxana import config
from gxana.stages import qfactors

from qfactors_helpers import gx_env

CFG = config.load_channel("kpkpxim")
ROOT = Path(__file__).resolve().parents[2]
MODELS = sorted(p.name[len("configPDFs_"):-2]
                for p in (ROOT / CFG["qfactors"]["config_dir"]).glob("configPDFs_*.h"))
pytestmark = [
    pytest.mark.skipif(not (ROOT / "packages/qfactors/main.C").is_file(), reason="packages/qfactors not checked out"),
    pytest.mark.skipif(shutil.which("root-config") is None, reason="ROOT not on PATH"),
]


@pytest.mark.parametrize("model", MODELS)
def test_model_compiles_with_engine(model, tmp_path):
    job = qfactors.plan_qfactors(CFG, "2017-01", model=model, environ=gx_env(tmp_path))
    qfactors.stage(job)
    qfactors.compile_main(job, syntax_only=True)
