# Market Platform

A C++20 market simulator, a REST API that serves its results, and a Dear ImGui dashboard that charts them.

```text
 experiments/*.json ──► market_simulation ──► output/<run>/market.csv
                                                    │
 SQLite (demo stocks) ─────────────┐                ▼
                                   └──────►  FastAPI  (api/)  ◄── JWT login
                                                    ▲
                                                    │ HTTP + JSON
                                           ImGui dashboard (apps/dashboard)
```

| Part | Where | What it does |
|---|---|---|
| Simulation library | `include/`, `src/`, `tests/` | Limit order book, agents, price models, statistics. See [docs/market-model.md](docs/market-model.md) |
| CLI | `examples/run_experiment.cpp` | `market_simulation <config.json> [output_dir]` |
| API | `api/` | Login, demo stock candles, simulation runs aggregated into candles |
| Dashboard | `apps/dashboard/` | Login screen, candlestick chart with volume and tooltip, stocks or simulation runs |
| Analysis | `analysis/analyze.py` | Text report and charts for a run |

## Quick start

```bash
# 1. Simulation library, CLI and tests
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure

# 2. Produce some data
./build/market_simulation experiments/baseline/config.json output/baseline
./build/market_simulation experiments/high_volatility/config.json output/high_volatility

# 3. API (Python 3.10+)
python3 -m venv api/.venv && source api/.venv/bin/activate
pip install -r api/requirements.txt
cd api && uvicorn main:app --host 127.0.0.1 --port 8000

# 4. Dashboard (new terminal; needs OpenGL, GLFW and libcurl dev packages)
scripts/setup_dependencies.sh
cmake -S . -B build -DBUILD_DASHBOARD=ON
cmake --build build
./build/apps/dashboard/market_dashboard
```

The development login is `admin` / `password`. Debian/Ubuntu packages: `libglfw3-dev libcurl4-openssl-dev libgl1-mesa-dev`.

Run the API tests with `cd api && pip install -r requirements-dev.txt && python -m pytest`.

## Configuration

Everything is configured through environment variables.

| Variable | Used by | Default |
|---|---|---|
| `MARKET_ENV` | API | `development`. With `production` the two secrets below are mandatory |
| `MARKET_JWT_SECRET` | API | random per process (tokens die on restart) |
| `MARKET_ADMIN_PASSWORD` | API | `password` (only applied when the admin user is first created) |
| `MARKET_DB` | API | `api/app.db` |
| `MARKET_OUTPUT_DIR` | API | `output/` |
| `MARKET_EXPERIMENTS_DIR` | API | `experiments/` |
| `MARKET_SIMULATION_BIN` | API | `build/market_simulation` |
| `MARKET_API_URL` | Dashboard | `http://127.0.0.1:8000` |
| `MARKET_API_DEBUG` | Dashboard | unset; set to log `METHOD url` (never request bodies) |

For a release build of the dashboard use `-DDASHBOARD_DEV_HINTS=OFF` to remove the pre-filled username and the credentials hint.

## API

| Endpoint | Auth | Description |
|---|---|---|
| `GET /health` | no | Liveness |
| `POST /login` | no | `{username, password}` → `{token, token_type, expires_in}`. 5 failures lock a username for 60 s (429) |
| `GET /stocks`, `GET /stocks/{symbol}/prices?limit=` | yes | Demo candles (AAPL, MSFT, NVDA are seeded) |
| `POST /stocks/{symbol}/prices` | yes | Add a candle (validated OHLC, 409 on duplicate timestamp) |
| `GET /simulations` | yes | `{experiments: [...], runs: [...]}` |
| `GET /simulations/{run}/candles?interval=N` | yes | `market.csv` aggregated to candles of N steps |
| `POST /simulations/{experiment}/run` | yes | Runs a named experiment with the C++ binary; only names found in `experiments/` are accepted |
| `GET/POST/PUT/DELETE /data` | yes | Small CRUD table kept from the original API |

Interactive docs: http://127.0.0.1:8000/docs

## Layout

```text
├── CMakeLists.txt          library, examples, tests, optional dashboard
├── include/market/ src/    simulation library
├── examples/ experiments/  CLI and JSON experiment configs
├── tests/ analysis/        C++ tests, Python analysis
├── api/                    FastAPI backend and its tests
├── apps/dashboard/         Dear ImGui client
├── scripts/                dependency setup
└── docs/market-model.md    full simulation documentation
```

## License

MIT. See `LICENSE`.
