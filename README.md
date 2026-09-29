# C++ Market Model

A small, modular C++20 framework for simulating a financial market with a limit order book, simulated traders and a pluggable model for the asset's fundamental value.

The core market mechanics (order book, matching, settlement) know nothing about trading strategies. Strategies live in agents, and the price dynamics live in replaceable models.

## Features

* Limit order book with price-time priority
* Limit and market orders, with partial fills
* Matching engine (trades execute at the resting order's price)
* Cash and asset reservation, so traders cannot spend the same funds twice
* Settlement of trades between traders
* Order registry (order status tracking) and trade history
* Market statistics: best bid/ask, mid price, spread, last price, volume, VWAP
* Agents: `RandomTrader` and `FundamentalTrader`
* Fundamental-value model with a replaceable `PriceModel` (a seeded `RandomWalkModel` is included)
* Step-based simulation loop
* Reproducible runs through explicit random seeds
* Unit tests for every component

## Project structure

```text
market-model/
├── CMakeLists.txt
├── include/market/
│   ├── core/          types.hpp, config.hpp, id_generator.hpp
│   ├── market/        order, trade, order_book, matching_engine, match_result,
│   │                  order_registry, trade_history, settlement, market
│   ├── agents/        agent, trader, trader_registry, random_trader, fundamental_trader
│   ├── models/        price_model, fundamental_value, random_walk_model
│   └── simulation/    simulation.hpp
├── src/               implementations, mirroring include/market/
├── examples/
│   ├── main.cpp               minimal hello-world executable
│   └── basic_simulation.cpp   runnable simulation demo
└── tests/
    ├── market/  agents/  models/
    └── types_test.cpp
```

## Architecture

```text
              ┌──────────────┐
              │  Simulation  │  clock, updates fundamental value, activates agents
              └──────┬───────┘
                     │ step()
              ┌──────▼───────┐        ┌────────────────────┐
              │    Agents    │───────▶│  FundamentalValue  │
              │ (strategies) │ reads  │  └─ PriceModel     │
              └──────┬───────┘        └────────────────────┘
                     │ submit_order()
              ┌──────▼───────────────────────────────┐
              │ Market                                │
              │  ├─ reserves cash / assets            │
              │  ├─ MatchingEngine ─▶ OrderBook       │
              │  ├─ OrderRegistry (order status)      │
              │  └─ TradeHistory, statistics          │
              └──────┬───────────────────────────────┘
                     │ trades
              ┌──────▼───────┐
              │  Settlement  │  moves cash and assets between TraderRegistry entries
              └──────────────┘
```

### Order flow

1. An agent builds an `Order` and calls `Market::submit_order`.
2. The market checks the order and **reserves** what it could cost: `price * quantity` in cash for a buy, or `quantity` assets for a sell. If the trader cannot afford it, the order is rejected and an empty vector is returned.
3. The `MatchingEngine` matches the order against the book. Any unfilled part of a limit order rests in the book. The unfilled part of a market order is discarded.
4. The market records the trades, updates the counterparties' orders, and returns any cash saved by price improvement, that is, buying cheaper than the limit price.
5. The agent passes each returned trade to `Settlement::settle`, which consumes the reserved cash and assets and moves them to the other side.

> **Note on market buy orders:** the `price` field of a market buy is the maximum price the trader will pay. This is needed to know how much cash to reserve, and the order will not trade above it. Market buys with `price <= 0` are rejected. Market sells ignore `price`.

## Building

### Requirements

* A C++20 compiler (GCC 10+, Clang 12+, MSVC 2019+)
* CMake 3.20+

### Configure and build

```bash
cmake -S . -B build
cmake --build build
```

Options:

| Option | Default | Meaning |
|---|---|---|
| `BUILD_TESTS` | `ON` | Build the unit tests |
| `BUILD_EXAMPLES` | `ON` | Build the example executables |

If you do not set `CMAKE_BUILD_TYPE`, the build defaults to `Debug`.

### Run the example

```bash
./build/basic_simulation
```

Example output:

```text
Agent activated at t = 1
...
Final time: 10
Final fundamental value: 98.5069
Trades executed: 18
```

On Windows with Visual Studio, executables are in `build/Debug/` (for example `build\Debug\basic_simulation.exe`).

### Run the tests

```bash
ctest --test-dir build --output-on-failure
```

The tests use `assert`, and the build turns off `NDEBUG` for them, so they also work in Release builds.

## Usage example

```cpp
#include <memory>

#include "market/agents/fundamental_trader.hpp"
#include "market/agents/random_trader.hpp"
#include "market/agents/trader_registry.hpp"
#include "market/core/config.hpp"
#include "market/market/market.hpp"
#include "market/models/fundamental_value.hpp"
#include "market/models/random_walk_model.hpp"
#include "market/simulation/simulation.hpp"

int main() {
    market::SimulationConfig config;
    config.steps = 1000;

    // Fundamental value follows a seeded random walk.
    market::RandomWalkModel model(/*drift*/ 0.0, /*volatility*/ 0.5, /*seed*/ 42);
    market::FundamentalValue fundamental(100.0, model);

    // Trader(id, cash, asset_quantity)
    market::TraderRegistry traders;
    traders.add_trader(market::Trader(1, 10000.0, 100));
    traders.add_trader(market::Trader(2, 10000.0, 100));

    market::Market market(traders);
    market::Simulation sim(config, market, fundamental, traders);

    // RandomTrader(id, reference_price, seed)
    sim.add_agent(std::make_unique<market::RandomTrader>(1, 100.0, 1));
    // FundamentalTrader(id, threshold, order_quantity)
    sim.add_agent(std::make_unique<market::FundamentalTrader>(2, 0.02, 5));

    sim.run();
}
```

### Writing your own agent

Implement `market::Agent`. On each step the agent gets the market, the trader registry, the settlement service and the current fundamental value:

```cpp
class MyAgent : public market::Agent {
public:
    void step(market::Timestamp t,
              market::Market& market,
              market::TraderRegistry& traders,
              market::Settlement& settlement,
              market::FundamentalValue& fundamental) override {
        market::Order order;
        order.trader_id = 1;
        order.side = market::Side::Buy;
        order.type = market::OrderType::Limit;
        order.price = 99.5;
        order.quantity = 10;
        order.timestamp = t;

        for (const auto& trade : market.submit_order(order)) {
            settlement.settle(trade);
        }
    }
};
```

Leave `order.id` as `0` to have the market assign a unique ID. Do not reserve cash or assets yourself, because the market does that.

### Writing your own price model

```cpp
class MyModel : public market::PriceModel {
public:
    market::Price next_price(market::Price current) override {
        return current * 1.001;
    }
};
```

Pass it to `FundamentalValue` in place of `RandomWalkModel`.

## Included agents and models

| Component | Behaviour |
|---|---|
| `RandomTrader` | Each step, buys or sells 1–10 units at the mid price (or the last trade price, or a reference price if the market is empty). |
| `FundamentalTrader` | Compares the market price with the fundamental value. Buys if the market is cheaper than fundamental by more than `threshold`, sells if it is more expensive, and otherwise holds. |
| `RandomWalkModel` | `price + drift + volatility * N(0, 1)`. It is additive, so prices can become negative with large volatility. |

## Design principles

* **Separation of concerns:** `Trader → Order → Market → Trade`. Agents never change prices directly.
* **Replaceable models:** price dynamics sit behind the `PriceModel` interface.
* **Deterministic core:** given the same sequence of orders, the market produces the same result. Randomness lives only in agents and models, and takes an explicit seed.
* **No global state:** the market, traders, registries and RNGs are passed in explicitly.

## Roadmap

Implemented:

- [x] `Order`, `Trade`, `OrderBook`, `MatchingEngine`, `Market`
- [x] Order registry and trade history
- [x] Cash and asset reservation, settlement
- [x] Trader accounts and registry
- [x] Random and fundamental traders
- [x] Fundamental value with a random-walk model
- [x] Simulation loop and simulation config
- [x] Unit tests

Ideas for the future:

- [ ] Market maker agent
- [ ] Momentum / noise / arbitrage traders
- [ ] Statistics module (returns, volatility, depth, P&L) and CSV output
- [ ] Configuration from JSON files
- [ ] Event scheduler and random agent activation
- [ ] Multiplicative price models, volatility models, news shocks
- [ ] Tick sizes, transaction costs, short selling, latency
- [ ] Python analysis scripts

## Disclaimer

This project is intended for research, experimentation and education. A simulated market is a simplified model and should not be assumed to reproduce real-world market behaviour or investment outcomes.

## License

MIT. See `LICENSE`.
