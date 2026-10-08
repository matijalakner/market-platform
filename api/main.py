"""FastAPI backend for the market dashboard.

Run from this directory:  uvicorn main:app --host 127.0.0.1 --port 8000
"""

import sqlite3
from contextlib import asynccontextmanager
from datetime import datetime, timezone
from typing import Annotated

from fastapi import APIRouter, Depends, FastAPI, HTTPException, Query
from fastapi import Path as PathParam
from pydantic import BaseModel, Field, field_validator, model_validator

import config
import simulations
from database import connection, initialize_database
from security import (DUMMY_HASH, create_token, current_user, login_throttle,
                      password_hash)


@asynccontextmanager
async def lifespan(_: FastAPI):
    initialize_database(password_hash.hash)
    yield


app = FastAPI(title="Market Dashboard API", version="2.0.0", lifespan=lifespan)
protected = APIRouter(dependencies=[Depends(current_user)])

SymbolParam = Annotated[str, PathParam(pattern=r"^[A-Za-z0-9.\-]{1,12}$")]
SimulationName = Annotated[str, PathParam(pattern=simulations.NAME_PATTERN)]


# ---- request models -------------------------------------------------------

class LoginRequest(BaseModel):
    username: str = Field(min_length=1, max_length=128)
    password: str = Field(min_length=1, max_length=256)


class DataRequest(BaseModel):
    name: str = Field(min_length=1, max_length=100)
    value: float = Field(allow_inf_nan=False)


class StockPriceRequest(BaseModel):
    timestamp: datetime
    open: float = Field(gt=0, allow_inf_nan=False)
    high: float = Field(gt=0, allow_inf_nan=False)
    low: float = Field(gt=0, allow_inf_nan=False)
    close: float = Field(gt=0, allow_inf_nan=False)
    volume: int = Field(default=0, ge=0)

    @field_validator("timestamp")
    @classmethod
    def to_naive_utc(cls, value: datetime) -> datetime:
        # One canonical form, so string ordering in SQLite is time ordering.
        if value.tzinfo is not None:
            value = value.astimezone(timezone.utc).replace(tzinfo=None)
        return value

    @model_validator(mode="after")
    def check_ohlc(self):
        if self.high < max(self.open, self.close):
            raise ValueError("high must be at least open and close")
        if self.low > min(self.open, self.close):
            raise ValueError("low must be at most open and close")
        return self


# ---- public routes --------------------------------------------------------

@app.get("/health")
def health():
    return {"status": "ok"}


@app.post("/login")
def login(request: LoginRequest):
    key = request.username.lower()
    wait = login_throttle.retry_after(key)
    if wait:
        raise HTTPException(
            429, f"Too many failed login attempts. Try again in {wait} s.",
            headers={"Retry-After": str(wait)})

    with connection() as conn:
        user = conn.execute(
            "SELECT username, password_hash FROM users WHERE username = ?",
            (request.username,),
        ).fetchone()

    stored = user["password_hash"] if user else DUMMY_HASH
    valid = password_hash.verify(request.password, stored)
    if user is None or not valid:
        login_throttle.record_failure(key)
        raise HTTPException(401, "Invalid username or password")

    login_throttle.record_success(key)
    return {
        "token": create_token(user["username"]),
        "token_type": "bearer",
        "expires_in": config.TOKEN_HOURS * 3600,
    }


# ---- generic data table ---------------------------------------------------

@protected.get("/data")
def list_data():
    with connection() as conn:
        rows = conn.execute("SELECT id, name, value FROM data ORDER BY id").fetchall()
    return [dict(row) for row in rows]


@protected.post("/data", status_code=201)
def create_data(request: DataRequest):
    with connection() as conn:
        cursor = conn.execute(
            "INSERT INTO data (name, value) VALUES (?, ?)", (request.name, request.value))
        new_id = cursor.lastrowid
    return {"id": new_id, "name": request.name, "value": request.value}


@protected.put("/data/{data_id}")
def update_data(data_id: int, request: DataRequest):
    with connection() as conn:
        cursor = conn.execute(
            "UPDATE data SET name = ?, value = ? WHERE id = ?",
            (request.name, request.value, data_id))
        found = cursor.rowcount > 0
    if not found:
        raise HTTPException(404, "Data record not found")
    return {"id": data_id, "name": request.name, "value": request.value}


@protected.delete("/data/{data_id}")
def delete_data(data_id: int):
    with connection() as conn:
        found = conn.execute("DELETE FROM data WHERE id = ?", (data_id,)).rowcount > 0
    if not found:
        raise HTTPException(404, "Data record not found")
    return {"deleted": True, "id": data_id}


# ---- demo stocks ----------------------------------------------------------

@protected.get("/stocks")
def list_symbols():
    with connection() as conn:
        rows = conn.execute(
            "SELECT DISTINCT symbol FROM stock_prices ORDER BY symbol").fetchall()
    return {"symbols": [row["symbol"] for row in rows]}


@protected.get("/stocks/{symbol}/prices")
def get_stock_prices(symbol: SymbolParam,
                     limit: int = Query(5000, ge=1, le=20_000)):
    symbol = symbol.upper()
    with connection() as conn:
        rows = conn.execute(
            """SELECT timestamp, open, high, low, close, volume FROM (
                   SELECT timestamp, open, high, low, close, volume
                   FROM stock_prices WHERE symbol = ?
                   ORDER BY timestamp DESC LIMIT ?
               ) ORDER BY timestamp""",
            (symbol, limit),
        ).fetchall()
    if not rows:
        raise HTTPException(404, f"No stock data found for {symbol}")
    return {"symbol": symbol, "source": "demo", "prices": [dict(row) for row in rows]}


@protected.post("/stocks/{symbol}/prices", status_code=201)
def add_stock_price(symbol: SymbolParam, request: StockPriceRequest):
    symbol = symbol.upper()
    timestamp = request.timestamp.isoformat(timespec="seconds")
    try:
        with connection() as conn:
            cursor = conn.execute(
                """INSERT INTO stock_prices
                   (symbol, timestamp, open, high, low, close, volume)
                   VALUES (?, ?, ?, ?, ?, ?, ?)""",
                (symbol, timestamp, request.open, request.high,
                 request.low, request.close, request.volume))
            new_id = cursor.lastrowid
    except sqlite3.IntegrityError:
        raise HTTPException(409, f"A candle for {symbol} at {timestamp} already exists")
    return {"id": new_id, "symbol": symbol, "timestamp": timestamp,
            "open": request.open, "high": request.high, "low": request.low,
            "close": request.close, "volume": request.volume}


# ---- simulation runs ------------------------------------------------------

@protected.get("/simulations")
def list_simulations():
    return {"experiments": simulations.experiment_names(),
            "runs": simulations.run_names()}


@protected.get("/simulations/{name}/candles")
def simulation_candles(name: SimulationName,
                       interval: int = Query(10, ge=1, le=10_000)):
    try:
        candles = simulations.read_candles(name, interval)
    except FileNotFoundError:
        raise HTTPException(404, f"No simulation output named {name}")
    if not candles:
        raise HTTPException(404, f"Simulation output {name} contains no usable rows")
    return {"symbol": name, "source": "simulation", "interval": interval,
            "prices": candles}


@protected.post("/simulations/{name}/run")
def run_simulation(name: SimulationName):
    try:
        return simulations.run_experiment(name)
    except FileNotFoundError:
        raise HTTPException(404, f"No experiment named {name}")
    except simulations.SimulationUnavailable as error:
        raise HTTPException(503, str(error))
    except simulations.SimulationBusy as error:
        raise HTTPException(409, str(error))
    except simulations.SimulationFailed as error:
        raise HTTPException(500, f"Simulation failed: {error}")


app.include_router(protected)
