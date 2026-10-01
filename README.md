# C++ Market Model

A modular C++20 framework for simulating a financial market: a limit order book, simulated traders with different strategies, replaceable price models, statistics and CSV output, and experiments driven by JSON files.

The core market mechanics (order book, matching, settlement) know nothing about trading strategies. Strategies live in agents, price dynamics live in models, and measurements live in the statistics module.

## Features

**Market**
* Limit order book with price-time priority, limit and market orders, partial fills
* Matching engine (trades execute at the resting order's price)
* Integer prices (`std::uint64_t` ticks) with a configurable tick size
* Cash and asset reservation, so funds can't be spent twice
* Settlement with transaction costs (fee rate)
* Short selling with a per-trader limit
* Order latency (orders reach the book N steps after submission)
* Order registry, trade history, VWAP, depth, spread, mid price

**Agents**
* `RandomTrader`: random side and size, quotes at the market price
* `NoiseTrader`: random limit orders around the price plus occasional market orders
* `FundamentalTrader`: trades toward the fundamental value
* `ArbitrageTrader`: takes quotes that are clearly away from fundamental value
* `MomentumTrader`: follows recent price moves
* `MarketMaker`: two-sided quotes with inventory skew

**Models**
* `RandomWalkModel`: additive random walk in ticks
* `GeometricBrownianModel`: multiplicative moves, scale with the price level
* `GarchModel`: volatility clustering (GARCH(1,1))
* News shocks (scheduled jumps in the fundamental value)

**Simulation and analysis**
* Step-based simulation with an event scheduler, random agent activation and random activation order
* Statistics: returns, volatility, spread, depth, volume, mispricing, per-trader inventory and P&L
* CSV output and a Python analysis script (text report and charts)
* JSON experiment configs with a ready-to-run runner
* Reproducible runs: every random source takes an explicit seed
* 30 unit tests, including a conservation test (money and assets balance after a busy run)

## Quick start

```bash
cmake -S . -B build
cmake --build build

# run an experiment and write CSVs to output/baseline
./build/market_simulation experiments/baseline/config.json

# analyse it
python3 analysis/analyze.py output/baseline
python3 analysis/analyze.py output/baseline --plot      # needs matplotlib

# tests
ctest --test-dir build --output-on-failure
```

On Windows with Visual Studio the executables are in `build/Debug/` or `build/Release/`. The default build type is `Release`; the tests keep `assert` enabled either way.

Options: `-DBUILD_TESTS=OFF`, `-DBUILD_EXAMPLES=OFF`.

### Executables

| Target | What it does |
|---|---|
| `market_simulation <config.json> [output_dir]` | Runs an experiment and prints a summary |
| `basic_simulation` | Minimal hand-built simulation in C++ |
| `market_model_demo` | Prints a hello message |

## Prices, ticks and money

Prices are integers: a number of **ticks**. The currency value of one tick is `tick_size` (default `0.01`), so `10025` ticks is `100.25`. Integer prices make order book keys exact, avoid floating-point comparisons, and rule out negative prices.

* `Price` and `Quantity` are `std::uint64_t`. `Position` (asset holding) is `std::int64_t`, negative when short.
* Cash and P&L are in **tick units** (price in ticks times quantity). The statistics module and the JSON config convert to and from currency for you.
* `market/core/units.hpp` has the conversions: `to_price` (rounds and clamps to at least 1 tick), `currency_to_ticks`, `ticks_to_currency`.

```cpp
market::Price p = market::currency_to_ticks(100.25, 0.01);   // 10025
market::Price q = market::to_price(10025.4 * 1.01);           // rounds, never < 1
```

## Architecture

```text
              ┌────────────────────┐
              │    Simulation      │  scheduler, activation, clock
              └────────┬───────────┘
        events/news    │ step()
              ┌────────▼───────┐        ┌────────────────────┐
              │     Agents     │───────▶│  FundamentalValue  │
              │  (strategies)  │ reads  │  └─ PriceModel     │
              └────────┬───────┘        └────────────────────┘
                       │ submit_order()
              ┌────────▼──────────────────────────────┐
              │ Market                                 │
              │  ├─ reserves cash / assets             │
              │  ├─ latency queue                      │
              │  ├─ MatchingEngine ─▶ OrderBook        │
              │  ├─ OrderRegistry, TradeHistory        │
              └────────┬──────────────────────────────┘
                       │ trades
              ┌────────▼───────┐        ┌────────────────────┐
              │   Settlement   │        │ StatisticsRecorder │ ─▶ CSV
              │  (cash, fees)  │        └────────────────────┘
              └────────────────┘
```

### Simulation step

1. Advance the clock.
2. Run scheduled events (news shocks, custom callbacks).
3. Update the fundamental value from its price model.
4. Match orders whose latency has elapsed and settle their trades.
5. Activate agents: all of them, or each with probability `activation_probability`, optionally in random order.
6. Record statistics.

### Order flow

1. An agent builds an `Order` and calls `Market::submit_order`.
2. The market **reserves** the worst-case cost (`price * quantity` cash for a buy, `quantity` assets for a sell). If the trader can't fund it, the order is rejected and `{}` is returned.
3. With zero latency the `MatchingEngine` matches immediately; with latency the order waits in a queue and `Market::process_pending` matches it on arrival.
4. Unfilled limit quantity rests in the book. The unfilled part of a market order is discarded.
5. Unused reservation (price improvement, finished or cancelled orders) is released.
6. The caller passes each returned trade to `Settlement::settle`, which moves cash and assets and charges fees. The `Simulation` settles trades from delayed orders itself.

> **Market buy orders:** `price` is the maximum price the trader will pay (needed to know how much cash to reserve). Market buys with `price == 0` are rejected. Market sells ignore `price`.

### Short selling

A trader created with `short_limit = N` may sell up to `N` assets it doesn't own. `available_assets()` is `position + short_limit - reserved`, so a trader holding 10 with a limit of 50 can sell 60 and ends at position -50. P&L is mark-to-market: `cash + position * price - initial equity`.

### Transaction costs

`simulation.fee_rate` is a fraction of trade value charged to both buyer and seller on every trade. Fees leave the system (`Settlement::total_fees()`), and a market maker's half-spread has to exceed them to be profitable.

### Latency

`market.latency = N` delays every order by N steps between submission and reaching the book. Funds are reserved at submission, and orders can be cancelled while queued.

## Experiments

An experiment is a JSON file. Everything except `simulation.steps` has a default, and money values are in currency:

```json
{
    "simulation": {
        "steps": 5000, "seed": 12345,
        "activation_probability": 0.5, "random_order": true,
        "fee_rate": 0.0
    },
    "market": { "initial_price": 100.0, "tick_size": 0.01, "latency": 0 },
    "traders": { "cash": 10000, "assets": 100, "short_limit": 0 },
    "model": { "type": "gbm", "mu": 0.0, "sigma": 0.001 },
    "agents": {
        "random_traders": 10, "fundamental_traders": 30, "market_makers": 2,
        "momentum_traders": 3, "noise_traders": 5, "arbitrage_traders": 10
    },
    "agent_params": {
        "order_quantity": 10, "market_maker_half_spread": 0.05,
        "arbitrage_threshold": 0.005
    },
    "news": [ { "time": 2000, "shock": -0.08 } ],
    "output": { "directory": "output/baseline", "trader_interval": 10 }
}
```

| Section | Keys |
|---|---|
| `simulation` | `steps`, `seed`, `activation_probability`, `random_order`, `fee_rate` |
| `market` | `initial_price`, `tick_size`, `latency` |
| `traders` | `cash`, `assets`, `short_limit` (same for every trader) |
| `model` | `type`: `random_walk` (`drift`, `volatility` in currency/step), `gbm` (`mu`, `sigma`), `garch` (`mu`, `omega`, `alpha`, `beta`) |
| `agents` | counts of `random_traders`, `fundamental_traders`, `market_makers`, `momentum_traders`, `noise_traders`, `arbitrage_traders` |
| `agent_params` | `fundamental_threshold`, `order_quantity`, `market_maker_half_spread`, `market_maker_quantity`, `market_maker_skew`, `momentum_lookback`, `momentum_threshold`, `noise_max_offset`, `noise_market_order_probability`, `arbitrage_threshold` |
| `news` | list of `{ "time", "shock" }`; shock is relative (`-0.08` = -8%) |
| `output` | `directory` (empty = no files), `trader_interval` (per-trader rows every N steps, 0 = none) |

Included experiments (in `experiments/`):

| Name | What it explores |
|---|---|
| `baseline` | Mixed population, GBM fundamental value |
| `high_volatility` | GARCH volatility clustering plus two news shocks |
| `many_market_makers` | Ten market makers instead of two (liquidity) |
| `high_trading_activity` | Many noisy agents, every agent active every step, fees, latency, short selling |

To compare conditions, copy a config, change a value and run it. Same config and seed always give the same result.

### Output files

`market.csv` (one row per step): `timestamp, price, fundamental, bid, ask, mid, spread, volume, total_volume, bid_depth, ask_depth`. `price` is the last trade, else mid, else fundamental value. Bid/ask/mid/spread are blank when a side of the book is empty.

`traders.csv`: `timestamp, trader_id, cash, inventory, pnl`. P&L is marked to the market price.

All values are in currency.

## Using it as a library

```cpp
#include <memory>

#include "market/agents/fundamental_trader.hpp"
#include "market/agents/market_maker.hpp"
#include "market/models/geometric_brownian_model.hpp"
#include "market/simulation/simulation.hpp"

int main() {
    market::SimulationConfig config;
    config.steps = 1000;
    config.fee_rate = 0.0005;

    market::GeometricBrownianModel model(0.0, 0.001, /*seed*/ 42);
    market::FundamentalValue fundamental(10000, model);          // 100.00

    market::TraderRegistry traders;                              // Trader(id, cash, assets, short_limit)
    traders.add_trader(market::Trader(1, 1'000'000.0, 100));
    traders.add_trader(market::Trader(2, 1'000'000.0, 100, 20));

    market::Market market(traders);
    market::Simulation sim(config, market, fundamental, traders);

    sim.add_agent(std::make_unique<market::MarketMaker>(1, /*half spread, ticks*/ 5, /*size*/ 5, /*skew*/ 0.5));
    sim.add_agent(std::make_unique<market::FundamentalTrader>(2, /*threshold*/ 0.02, /*size*/ 5));
    sim.schedule_news(500, -0.05);                               // -5% shock at step 500

    market::StatisticsRecorder recorder(config.tick_size);
    sim.set_recorder(&recorder);

    sim.run();

    recorder.write_market_csv("market.csv");
    auto summary = recorder.summary();                           // volatility, spread, depth...
}
```

### Writing your own agent

Implement `market::Agent`. Leave `order.id` as `0` so the market assigns one, and don't reserve funds yourself, because the market does that:

```cpp
class MyAgent : public market::Agent {
public:
    void step(market::Timestamp t, market::Market& market,
              market::TraderRegistry& traders, market::Settlement& settlement,
              market::FundamentalValue& fundamental) override {
        market::Order order;
        order.trader_id = 1;
        order.side = market::Side::Buy;
        order.type = market::OrderType::Limit;
        order.price = market::reference_price(market, fundamental.value());  // ticks
        order.quantity = 10;
        order.timestamp = t;

        for (const auto& trade : market.submit_order(order)) {
            settlement.settle(trade);
        }
    }
};
```

Useful helpers: `market::reference_price(market, fallback)` (`agents/agent_utils.hpp`), `market.cancel_all_orders(trader_id)`, `market.best_bid()/best_ask()/mid_price()`.

### Writing your own price model

```cpp
class MyModel : public market::PriceModel {
public:
    market::Price next_price(market::Price current) override {
        return market::to_price(current * 1.0005);   // always return a valid price
    }
};
```

## Included agents

| Agent | Behaviour |
|---|---|
| `RandomTrader(id, reference_price, seed)` | Random side and size 1–10 at the market price |
| `NoiseTrader(id, reference_price, seed, max_offset, max_quantity, market_order_probability)` | Random limit orders within `max_offset` ticks, sometimes market orders |
| `FundamentalTrader(id, threshold, quantity)` | Buys when the market is cheaper than fundamental by more than `threshold`, sells when dearer |
| `ArbitrageTrader(id, threshold, quantity)` | Takes the best ask/bid when it is more than `threshold` away from fundamental |
| `MomentumTrader(id, lookback, threshold, quantity, fallback_price)` | Buys after a rise over `lookback` steps, sells after a fall |
| `MarketMaker(id, half_spread, quantity, skew)` | Cancels and re-posts a bid and ask each step, shifted against inventory |

`templates/market_maker_template.hpp` is a more configurable market-maker base class with volatility-adaptive spreads, inventory caps, multiple levels and a kill switch.

## Models

| Model | Formula (per step) | Notes |
|---|---|---|
| `RandomWalkModel(drift, volatility, seed)` | `p + drift + vol * N(0,1)` | Units are ticks. Floors at 1 tick. |
| `GeometricBrownianModel(mu, sigma, seed)` | `p * exp(mu - sigma²/2 + sigma * N(0,1))` | Moves scale with price. |
| `GarchModel(mu, omega, alpha, beta, seed)` | `p * exp(mu + sigma_t * z)`, `sigma²_t+1 = omega + alpha*shock² + beta*sigma²_t` | Volatility clusters. Needs `alpha + beta < 1`. |

Prices are rounded to whole ticks after every step, so very small per-step moves can round to zero. Use a smaller tick size or larger volatility if a price looks frozen.

## Analysis

```bash
python3 analysis/analyze.py output/baseline output/high_volatility
python3 analysis/analyze.py output/baseline --plot
```

The text report (standard library only) gives return, max drawdown, volatility, return autocorrelation, spread, depth, volume, mean mispricing and trader P&L. `--plot` (needs `matplotlib`) saves `market.png` (price vs fundamental, spread, volume, depth) and `traders.png` (P&L and inventory).

## Project structure

```text
market-model/
├── CMakeLists.txt
├── include/market/
│   ├── core/          types, units, config, id_generator, json
│   ├── market/        order, trade, order_book, matching_engine, match_result,
│   │                  order_registry, trade_history, settlement, market
│   ├── agents/        agent, agent_utils, trader, trader_registry, random_trader,
│   │                  noise_trader, fundamental_trader, arbitrage_trader,
│   │                  momentum_trader, market_maker
│   ├── models/        price_model, fundamental_value, random_walk_model,
│   │                  geometric_brownian_model, garch_model
│   ├── statistics/    statistics.hpp
│   └── simulation/    simulation, event_scheduler, experiment
├── src/               implementations, mirroring include/market/
├── examples/          run_experiment.cpp, basic_simulation.cpp, main.cpp
├── experiments/       baseline/, high_volatility/, many_market_makers/, high_trading_activity/
├── analysis/          analyze.py, requirements.txt
└── tests/             core/ market/ agents/ models/ statistics/ simulation/
```

## Design principles

* **Separation of concerns:** `Trader → Order → Market → Trade`. Agents never change prices directly.
* **Replaceable models and agents** behind small interfaces.
* **Deterministic core:** the same orders give the same result. Randomness lives in agents, models and activation, and always takes a seed.
* **Integer prices:** exact comparisons, no negative prices.
* **No global state.**

## Possible future work

* Price-impact and order-arrival-rate statistics
* Parallel Monte Carlo runs over many seeds
* Multiple assets or exchanges
* Dividends, borrowing costs and leverage
* Calibration against historical data
* Reinforcement-learning agents

## Disclaimer

This project is intended for research, experimentation and education. A simulated market is a simplified model and should not be assumed to reproduce real-world market behaviour or investment outcomes.

## License

MIT. See `LICENSE`.
