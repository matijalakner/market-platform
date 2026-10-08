import os
import tempfile
from pathlib import Path

# Must be set before any application module is imported.
_tmp = Path(tempfile.mkdtemp(prefix="market-api-test-"))
os.environ.update({
    "MARKET_DB": str(_tmp / "test.db"),
    "MARKET_OUTPUT_DIR": str(_tmp / "output"),
    "MARKET_EXPERIMENTS_DIR": str(_tmp / "experiments"),
    "MARKET_JWT_SECRET": "test-secret-" + "x" * 40,
    "MARKET_ADMIN_PASSWORD": "test-password",
})

import pytest  # noqa: E402
from fastapi.testclient import TestClient  # noqa: E402

import security  # noqa: E402
from main import app  # noqa: E402


@pytest.fixture(autouse=True)
def _reset_throttle():
    security.login_throttle.reset()


@pytest.fixture(scope="session")
def client():
    with TestClient(app) as test_client:  # runs the lifespan (DB init)
        yield test_client


@pytest.fixture()
def auth(client):
    response = client.post("/login", json={"username": "admin", "password": "test-password"})
    assert response.status_code == 200
    token = response.json()["token"]
    return {"Authorization": f"Bearer {token}"}


@pytest.fixture()
def output_dir():
    return _tmp / "output"
