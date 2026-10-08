"""Runtime configuration, read once from environment variables."""

import logging
import os
import secrets
from pathlib import Path

log = logging.getLogger("market.api")

REPO_ROOT = Path(__file__).resolve().parent.parent
ENVIRONMENT = os.getenv("MARKET_ENV", "development").lower()
IS_PRODUCTION = ENVIRONMENT == "production"


def _from_env(name: str) -> str | None:
    value = os.getenv(name)
    if not value and IS_PRODUCTION:
        raise RuntimeError(f"{name} must be set when MARKET_ENV=production")
    return value or None


SECRET_KEY = _from_env("MARKET_JWT_SECRET")
if SECRET_KEY is None:
    SECRET_KEY = secrets.token_urlsafe(48)
    log.warning("MARKET_JWT_SECRET is not set: using a random key, "
                "issued tokens stop working when the server restarts")

ADMIN_PASSWORD = _from_env("MARKET_ADMIN_PASSWORD")
if ADMIN_PASSWORD is None:
    ADMIN_PASSWORD = "password"
    log.warning("MARKET_ADMIN_PASSWORD is not set: the admin account uses the "
                "insecure development password")

ALGORITHM = "HS256"
TOKEN_HOURS = int(os.getenv("MARKET_TOKEN_HOURS", "8"))

LOGIN_MAX_FAILURES = 5
LOGIN_LOCKOUT_SECONDS = 60

DATABASE = Path(os.getenv("MARKET_DB", Path(__file__).resolve().parent / "app.db"))
OUTPUT_DIR = Path(os.getenv("MARKET_OUTPUT_DIR", REPO_ROOT / "output"))
EXPERIMENTS_DIR = Path(os.getenv("MARKET_EXPERIMENTS_DIR", REPO_ROOT / "experiments"))
SIMULATION_BIN = Path(os.getenv("MARKET_SIMULATION_BIN",
                                REPO_ROOT / "build" / "market_simulation"))
SIMULATION_TIMEOUT_SECONDS = int(os.getenv("MARKET_SIMULATION_TIMEOUT", "300"))
