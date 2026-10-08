HEADER = "timestamp,price,fundamental,bid,ask,mid,spread,volume,total_volume,bid_depth,ask_depth\n"


def test_health(client):
    assert client.get("/health").json() == {"status": "ok"}


def test_login_success_and_failure(client):
    ok = client.post("/login", json={"username": "admin", "password": "test-password"})
    assert ok.status_code == 200 and ok.json()["token_type"] == "bearer"
    bad = client.post("/login", json={"username": "admin", "password": "nope"})
    assert bad.status_code == 401
    unknown = client.post("/login", json={"username": "ghost", "password": "nope"})
    assert unknown.status_code == 401


def test_login_is_throttled(client):
    for _ in range(5):
        assert client.post("/login", json={"username": "mallory", "password": "x"}).status_code == 401
    blocked = client.post("/login", json={"username": "mallory", "password": "x"})
    assert blocked.status_code == 429 and "retry-after" in blocked.headers


def test_protected_routes_require_a_token(client):
    assert client.get("/stocks/AAPL/prices").status_code == 401
    assert client.get("/stocks/AAPL/prices", headers={"Authorization": "Bearer junk"}).status_code == 401


def test_seeded_prices(client, auth):
    body = client.get("/stocks/aapl/prices", headers=auth).json()
    assert body["symbol"] == "AAPL" and len(body["prices"]) == 78
    for c in body["prices"]:
        assert c["low"] <= min(c["open"], c["close"]) and c["high"] >= max(c["open"], c["close"])


def test_unknown_and_malformed_symbols(client, auth):
    assert client.get("/stocks/ZZZZ/prices", headers=auth).status_code == 404
    assert client.get("/stocks/bad!/prices", headers=auth).status_code == 422


def test_add_price_validation_and_duplicates(client, auth):
    candle = {"timestamp": "2026-10-08T09:30:00", "open": 10, "high": 11, "low": 9, "close": 10.5, "volume": 5}
    assert client.post("/stocks/TEST/prices", json=candle, headers=auth).status_code == 201
    assert client.post("/stocks/TEST/prices", json=candle, headers=auth).status_code == 409
    bad = dict(candle, timestamp="2026-10-08T09:35:00", high=9.5)
    assert client.post("/stocks/TEST/prices", json=bad, headers=auth).status_code == 422


def test_simulation_candles(client, auth, output_dir):
    run = output_dir / "demo_run"
    run.mkdir(parents=True, exist_ok=True)
    rows = [(1, 100, 1), (2, 101, 2), (3, 99, 3), (4, 100, 4)]
    (run / "market.csv").write_text(
        HEADER + "".join(f"{t},{p},100,,,,,{v},0,0,0\n" for t, p, v in rows))

    assert "demo_run" in client.get("/simulations", headers=auth).json()["runs"]
    body = client.get("/simulations/demo_run/candles?interval=2", headers=auth).json()
    assert body["prices"] == [
        {"timestamp": "t=1", "open": 100, "high": 101, "low": 100, "close": 101, "volume": 3},
        {"timestamp": "t=3", "open": 99, "high": 100, "low": 99, "close": 100, "volume": 7},
    ]
    assert client.get("/simulations/missing/candles", headers=auth).status_code == 404
    assert client.get("/simulations/..%2Fetc/candles", headers=auth).status_code in (404, 422)


def test_run_unknown_experiment(client, auth):
    assert client.post("/simulations/nope/run", headers=auth).status_code == 404
