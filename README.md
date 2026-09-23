# C++ Market Model

A modular C++ framework for simulating financial markets, including order books, traders, market-making strategies, price models, and market dynamics.

The project is designed to make it easy to experiment with different market structures and trading-agent behaviors while keeping the core market mechanics independent from the models being tested.

## Features

* Limit order book
* Market and limit orders
* Order matching engine
* Multiple simulated traders
* Configurable trading strategies
* Fundamental-value models
* Price and volatility models
* Market-making agents
* Event-driven simulation
* Configurable simulation parameters
* Market statistics and performance metrics
* Reproducible simulations through configurable random seeds
* Unit tests for core components

## Project Structure

```text
market-model/
├── CMakeLists.txt
├── README.md
├── LICENSE
│
├── include/
│   └── market/
│       ├── core/
│       │   ├── types.hpp
│       │   ├── time.hpp
│       │   └── config.hpp
│       │
│       ├── market/
│       │   ├── market.hpp
│       │   ├── order_book.hpp
│       │   ├── order.hpp
│       │   ├── trade.hpp
│       │   └── matching_engine.hpp
│       │
│       ├── agents/
│       │   ├── agent.hpp
│       │   ├── random_trader.hpp
│       │   ├── fundamental_trader.hpp
│       │   └── market_maker.hpp
│       │
│       ├── models/
│       │   ├── price_model.hpp
│       │   ├── volatility_model.hpp
│       │   └── fundamental_model.hpp
│       │
│       ├── simulation/
│       │   ├── simulation.hpp
│       │   ├── event_queue.hpp
│       │   └── scheduler.hpp
│       │
│       └── statistics/
│           ├── statistics.hpp
│           ├── returns.hpp
│           └── metrics.hpp
│
├── src/
│   ├── market/
│   ├── agents/
│   ├── models/
│   ├── simulation/
│   └── statistics/
│
├── tests/
│   ├── market/
│   ├── agents/
│   └── models/
│
├── examples/
│   ├── basic_market.cpp
│   ├── agent_market.cpp
│   └── market_maker.cpp
│
├── configs/
│   ├── basic.json
│   └── agents.json
│
├── data/
│   ├── input/
│   └── output/
│
└── scripts/
    ├── run_simulation.py
    └── analyze_results.py
```

## Architecture

The model is divided into several independent layers:

```text
                    ┌─────────────────┐
                    │   Simulation    │
                    └────────┬────────┘
                             │
                    ┌────────▼────────┐
                    │ Event Scheduler │
                    └────────┬────────┘
                             │
                 ┌───────────▼───────────┐
                 │         Market        │
                 │                       │
                 │    Order Book         │
                 │    Matching Engine    │
                 │    Trade Generation   │
                 └───────────┬───────────┘
                             │
             ┌───────────────┼───────────────┐
             │               │               │
             ▼               ▼               ▼
        ┌──────────┐   ┌──────────┐   ┌────────────┐
        │ Trader A │   │ Trader B │   │ Market     │
        │          │   │          │   │ Maker      │
        └──────────┘   └──────────┘   └────────────┘
```

### Market

The market is responsible for the mechanics of trading.

Typical components include:

* `Order`
* `OrderBook`
* `MatchingEngine`
* `Trade`
* `Market`

The market should not need to know why a trader decided to submit an order.

### Agents

Agents represent participants in the market.

Examples:

* Random traders
* Fundamental traders
* Momentum traders
* Noise traders
* Arbitrage traders
* Market makers

Each agent can observe market information and generate orders according to its strategy.

### Models

Models describe the underlying dynamics of the simulated market.

Possible models include:

* Fundamental value
* Volatility
* Price dynamics
* Order arrival
* News/information
* Liquidity

Models should be replaceable without requiring changes to the core matching engine.

### Simulation

The simulation layer controls the passage of time and execution of events.

Responsibilities include:

* Simulation clock
* Event scheduling
* Agent activation
* Market updates
* Random-number generation
* Experiment configuration
* Simulation termination

### Statistics

The statistics layer collects and calculates market-level measurements.

Examples:

* Mid price
* Last traded price
* Returns
* Volatility
* Trading volume
* Bid/ask spread
* Market depth
* Price impact
* Trader P&L
* Inventory
* Order arrival rates

## Basic Simulation Flow

A typical simulation step looks like:

```text
1. Advance simulation clock
          │
          ▼
2. Update fundamental / market state
          │
          ▼
3. Select active trader(s)
          │
          ▼
4. Trader observes market
          │
          ▼
5. Trader generates order
          │
          ▼
6. Order enters order book
          │
          ▼
7. Matching engine executes orders
          │
          ▼
8. Trades update positions and cash
          │
          ▼
9. Record market statistics
          │
          ▼
10. Continue to next event
```

## Example Order

A basic order can contain:

```cpp
struct Order
{
    uint64_t id;
    uint64_t trader_id;

    Side side;
    OrderType type;

    double price;
    uint64_t quantity;

    uint64_t timestamp;
};
```

Possible order types:

```cpp
enum class OrderType
{
    Market,
    Limit
};
```

Possible sides:

```cpp
enum class Side
{
    Buy,
    Sell
};
```

## Example Trader Interface

Strategies should implement a common interface so that different types of traders can be substituted easily.

```cpp
class Trader
{
public:
    virtual ~Trader() = default;

    virtual Order generate_order(
        const MarketState& market
    ) = 0;
};
```

For example:

```cpp
class RandomTrader : public Trader
{
public:
    Order generate_order(
        const MarketState& market
    ) override;
};
```

This allows the simulation to operate on traders without knowing which strategy they use.

## Building

The project uses CMake.

### Requirements

* C++20-compatible compiler
* CMake 3.20+
* Git
* Optional: Python 3.x for analysis scripts

### Configure

```bash
cmake -S . -B build
```

### Build

```bash
cmake --build build
```

### Run

For example:

```bash
./build/market_simulation
```

The exact executable name may depend on the CMake configuration.

## Running Tests

Build the project with tests enabled:

```bash
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build
```

Run:

```bash
ctest --test-dir build
```

## Configuration

Simulation parameters should preferably be stored outside the C++ source code.

Example:

```json
{
    "simulation": {
        "steps": 100000,
        "seed": 12345
    },

    "market": {
        "initial_price": 100.0,
        "tick_size": 0.01
    },

    "agents": {
        "random_traders": 100,
        "fundamental_traders": 50,
        "market_makers": 5
    }
}
```

Keeping configuration separate makes it easier to run multiple experiments without recompiling the application.

## Reproducibility

Every simulation should support an explicit random seed.

For example:

```cpp
std::mt19937_64 rng(seed);
```

Using the same:

* configuration
* initial state
* random seed
* simulation version

should produce the same simulation results, assuming deterministic execution.

## Experiments

Experiments can be organized by configuration rather than by modifying source code.

For example:

```text
experiments/
├── baseline/
│   └── config.json
├── high_volatility/
│   └── config.json
├── many_market_makers/
│   └── config.json
└── high_trading_activity/
    └── config.json
```

This makes it possible to compare different market conditions systematically.

## Output

Simulation output can be written to CSV or another machine-readable format.

Example:

```text
timestamp,price,volume,bid,ask,spread
0,100.00,0,99.99,100.01,0.02
1,100.01,10,100.00,100.02,0.02
2,100.03,5,100.02,100.04,0.02
```

Trader-level output could contain:

```text
timestamp,trader_id,cash,inventory,pnl
0,1,10000,0,0
1,1,9990,100,0
2,1,10005,50,15
```

## Design Principles

### Separation of concerns

The exchange should not contain trading-strategy logic.

```text
Trader → Order → Market → Trade
```

Rather than:

```text
Trader → directly modify price
```

### Replaceable models

Models should use interfaces where appropriate so that implementations can be swapped.

For example:

```cpp
class PriceModel
{
public:
    virtual ~PriceModel() = default;

    virtual double next_price(
        double current_price,
        double dt
    ) = 0;
};
```

### Deterministic core

The core market engine should be deterministic given the same sequence of orders.

This makes debugging and testing substantially easier.

### Minimal global state

Avoid global variables for:

* prices
* orders
* traders
* random-number generators
* simulation time

Pass state explicitly through the relevant components.

## Development Roadmap

### Phase 1 — Core Market

* [ ] `Order`
* [ ] `Trade`
* [ ] `OrderBook`
* [ ] `MatchingEngine`
* [ ] Basic market
* [ ] Unit tests

### Phase 2 — Simulation

* [ ] Simulation clock
* [ ] Event queue
* [ ] Random-number management
* [ ] Configuration system
* [ ] Simulation loop

### Phase 3 — Agents

* [ ] Base trader
* [ ] Random trader
* [ ] Fundamental trader
* [ ] Market maker
* [ ] Trader positions
* [ ] Cash/P&L accounting

### Phase 4 — Market Models

* [ ] Fundamental value
* [ ] Volatility
* [ ] News events
* [ ] Order-arrival process
* [ ] Alternative price models

### Phase 5 — Analysis

* [ ] Return calculation
* [ ] Volatility statistics
* [ ] Liquidity metrics
* [ ] Spread analysis
* [ ] Volume analysis
* [ ] P&L analysis
* [ ] Python visualization tools

## Possible Future Extensions

The architecture can later be extended to support:

* Multiple assets
* Multiple exchanges
* Cross-asset trading
* Transaction costs
* Latency
* Partial order fills
* Order cancellation
* Order expiration
* Short selling
* Borrowing/leverage
* Dividends
* News shocks
* Limit-order-book reconstruction
* Reinforcement-learning agents
* Calibration against historical data
* Parallel Monte Carlo experiments

## Disclaimer

This project is intended for research, experimentation, and educational purposes. A simulated market model is necessarily a simplified representation of real financial markets and should not be assumed to reproduce real-world market behavior or investment outcomes.

