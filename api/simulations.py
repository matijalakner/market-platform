"""Bridge to the C++ market simulation: list runs, aggregate market.csv into
candles, and (optionally) run a named experiment with the market_simulation
binary. Only names that match NAME_PATTERN and exist on disk are ever used, so
clients cannot make the server touch arbitrary paths or run arbitrary programs.
"""

import csv
import math
import re
import subprocess
import threading
from pathlib import Path

import config

NAME_PATTERN = r"^[A-Za-z0-9_-]{1,64}$"
_NAME_RE = re.compile(NAME_PATTERN)
_run_lock = threading.Lock()


class SimulationUnavailable(Exception):
    """The market_simulation binary is missing."""


class SimulationBusy(Exception):
    """Another experiment is already running."""


class SimulationFailed(Exception):
    """The binary ran but failed."""


def _check_name(name: str) -> None:
    if not _NAME_RE.match(name):
        raise ValueError(f"invalid name: {name!r}")


def _subdirs_with(root: Path, filename: str) -> list[str]:
    if not root.is_dir():
        return []
    return sorted(
        p.name for p in root.iterdir()
        if p.is_dir() and _NAME_RE.match(p.name) and (p / filename).is_file()
    )


def experiment_names() -> list[str]:
    return _subdirs_with(config.EXPERIMENTS_DIR, "config.json")


def run_names() -> list[str]:
    return _subdirs_with(config.OUTPUT_DIR, "market.csv")


def read_candles(name: str, interval: int) -> list[dict]:
    """Aggregates market.csv into OHLC candles of `interval` simulation steps."""
    _check_name(name)
    path = config.OUTPUT_DIR / name / "market.csv"
    if not path.is_file():
        raise FileNotFoundError(name)

    candles: list[dict] = []
    current: dict | None = None
    current_bucket = -1

    with path.open(newline="") as handle:
        for row in csv.DictReader(handle):
            try:
                step = int(row["timestamp"])
                price = float(row["price"])
                volume = int(float(row.get("volume") or 0))
            except (KeyError, ValueError):
                continue
            if not math.isfinite(price):
                continue

            bucket = max(0, step - 1) // interval
            if current is None or bucket != current_bucket:
                if current is not None:
                    candles.append(current)
                current_bucket = bucket
                current = {"timestamp": f"t={step}", "open": price, "high": price,
                           "low": price, "close": price, "volume": 0}
            current["high"] = max(current["high"], price)
            current["low"] = min(current["low"], price)
            current["close"] = price
            current["volume"] += volume

    if current is not None:
        candles.append(current)
    for candle in candles:
        for key in ("open", "high", "low", "close"):
            candle[key] = round(candle[key], 4)
    return candles


def find_binary() -> Path:
    base = config.SIMULATION_BIN
    candidates = [base, base.with_suffix(".exe"),
                  base.parent / "Release" / base.name,
                  base.parent / "Release" / (base.name + ".exe")]
    for candidate in candidates:
        if candidate.is_file():
            return candidate
    raise SimulationUnavailable(
        f"market_simulation binary not found at {base}; build the project "
        "or set MARKET_SIMULATION_BIN")


def run_experiment(name: str) -> dict:
    _check_name(name)
    config_path = config.EXPERIMENTS_DIR / name / "config.json"
    if not config_path.is_file():
        raise FileNotFoundError(name)
    binary = find_binary()

    if not _run_lock.acquire(blocking=False):
        raise SimulationBusy("another simulation is already running")
    try:
        config.OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
        # The output directory is passed explicitly, so results never depend
        # on the server's working directory.
        process = subprocess.run(
            [str(binary), str(config_path), str(config.OUTPUT_DIR / name)],
            capture_output=True, text=True, check=False,
            timeout=config.SIMULATION_TIMEOUT_SECONDS,
        )
    except subprocess.TimeoutExpired:
        raise SimulationFailed("simulation timed out")
    finally:
        _run_lock.release()

    if process.returncode != 0:
        raise SimulationFailed(process.stderr.strip() or f"exit code {process.returncode}")
    return {"name": name, "summary": process.stdout.strip()}
