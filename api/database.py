"""SQLite access, schema and demo data."""

import math
import random
import sqlite3
from contextlib import contextmanager
from datetime import datetime, timedelta
from typing import Callable, Iterator

import config

DEMO_SYMBOLS = {"AAPL": 225.10, "MSFT": 415.00, "NVDA": 130.00}
DEMO_START = datetime(2026, 10, 7, 9, 30)
DEMO_CANDLES = 78  # one 5-minute trading day

SCHEMA = """
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS data (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    value REAL NOT NULL
);
CREATE TABLE IF NOT EXISTS stock_prices (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    symbol TEXT NOT NULL,
    timestamp TEXT NOT NULL,
    open REAL NOT NULL,
    high REAL NOT NULL,
    low REAL NOT NULL,
    close REAL NOT NULL,
    volume INTEGER NOT NULL DEFAULT 0
);
CREATE UNIQUE INDEX IF NOT EXISTS idx_stock_prices_symbol_ts
    ON stock_prices (symbol, timestamp);
"""


@contextmanager
def connection() -> Iterator[sqlite3.Connection]:
    """One connection per use: committed on success, rolled back on error,
    always closed (the original code leaked connections on exceptions)."""
    conn = sqlite3.connect(config.DATABASE)
    conn.row_factory = sqlite3.Row
    try:
        yield conn
        conn.commit()
    except Exception:
        conn.rollback()
        raise
    finally:
        conn.close()


def _demo_candles(symbol: str, start_price: float) -> list[tuple]:
    rng = random.Random(symbol)  # local, deterministic; no global random.seed()
    price = start_price
    rows = []
    for index in range(DEMO_CANDLES):
        timestamp = DEMO_START + timedelta(minutes=5 * index)
        open_price = price
        drift = 0.03 + math.sin(index / 8.0) * 0.08
        close_price = max(1.0, open_price + drift + rng.uniform(-0.32, 0.32))
        high_price = max(open_price, close_price) + rng.uniform(0.04, 0.28)
        low_price = max(0.01, min(open_price, close_price) - rng.uniform(0.04, 0.28))
        rows.append((
            symbol,
            timestamp.isoformat(timespec="seconds"),
            round(open_price, 4),
            round(high_price, 4),
            round(low_price, 4),
            round(close_price, 4),
            rng.randint(120_000, 650_000),
        ))
        price = close_price
    return rows


def initialize_database(hash_password: Callable[[str], str]) -> None:
    config.DATABASE.parent.mkdir(parents=True, exist_ok=True)
    with connection() as conn:
        conn.executescript(SCHEMA)

        if conn.execute("SELECT 1 FROM users WHERE username = 'admin'").fetchone() is None:
            conn.execute(
                "INSERT INTO users (username, password_hash) VALUES (?, ?)",
                ("admin", hash_password(config.ADMIN_PASSWORD)),
            )

        if conn.execute("SELECT COUNT(*) FROM data").fetchone()[0] == 0:
            conn.executemany(
                "INSERT INTO data (name, value) VALUES (?, ?)",
                [("Temperature", 23.5), ("Pressure", 1013.2), ("Humidity", 45.0)],
            )

        for symbol, start_price in DEMO_SYMBOLS.items():
            exists = conn.execute(
                "SELECT 1 FROM stock_prices WHERE symbol = ? LIMIT 1", (symbol,)
            ).fetchone()
            if exists is None:
                conn.executemany(
                    """INSERT INTO stock_prices
                       (symbol, timestamp, open, high, low, close, volume)
                       VALUES (?, ?, ?, ?, ?, ?, ?)""",
                    _demo_candles(symbol, start_price),
                )
