#include <iostream>
#include <memory>

#include "market/agents/fundamental_trader.hpp"
#include "market/agents/market_maker.hpp"
#include "market/agents/random_trader.hpp"
#include "market/agents/trader_registry.hpp"
#include "market/core/config.hpp"
#include "market/market/market.hpp"
#include "market/market/settlement.hpp"
#include "market/models/fundamental_value.hpp"
#include "market/models/random_walk_model.hpp"
#include "market/simulation/simulation.hpp"

class TestAgent : public market::Agent {
public:
    void step(
        market::Timestamp timestamp,
        market::Market& market,
        market::TraderRegistry& traders,
        market::Settlement& settlement,
        market::FundamentalValue& fundamental_value
    ) override {
        (void)market;
        (void)traders;
        (void)settlement;
        (void)fundamental_value;

        std::cout << "Agent activated at t = " << timestamp << '\n';
    }
};

int main() {
    market::SimulationConfig config;
    config.steps = 10;

    // Prices are integer ticks: 10000 ticks = 100.00 with a tick size of 0.01.
    market::RandomWalkModel price_model(/*drift*/ 10.0, /*volatility*/ 100.0, 12345);
    market::FundamentalValue fundamental_value(10000, price_model);

    // Cash is in tick units (price in ticks * quantity).
    market::TraderRegistry traders;
    traders.add_trader(market::Trader(1, 1000000.0, 100));
    traders.add_trader(market::Trader(2, 1000000.0, 100));
    traders.add_trader(market::Trader(3, 1000000.0, 100));
    traders.add_trader(market::Trader(4, 1000000.0, 100));

    market::Market market(traders);
    market::Simulation simulation(config, market, fundamental_value, traders);

    simulation.add_agent(std::make_unique<TestAgent>());
    simulation.add_agent(std::make_unique<market::MarketMaker>(1, /*half spread*/ 20, 5, 0.5));
    simulation.add_agent(std::make_unique<market::RandomTrader>(2, 10000, 2));
    simulation.add_agent(std::make_unique<market::RandomTrader>(3, 10000, 3));
    simulation.add_agent(std::make_unique<market::FundamentalTrader>(4, 0.02, 5));
    simulation.run();

    std::cout << "Final time: " << simulation.current_time() << '\n';
    std::cout << "Final fundamental value (ticks): " << simulation.fundamental_value() << '\n';
    std::cout << "Trades executed: " << market.trade_count() << '\n';

    return 0;
}
