#include <iostream>
#include <memory>

#include "market/agents/fundamental_trader.hpp"
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

    market::RandomWalkModel price_model(0.1, 1.0, 12345);
    market::FundamentalValue fundamental_value(100.0, price_model);

    market::TraderRegistry traders;
    traders.add_trader(market::Trader(1, 10000.0, 100));
    traders.add_trader(market::Trader(2, 10000.0, 100));
    traders.add_trader(market::Trader(3, 10000.0, 100));

    market::Market market(traders);
    market::Simulation simulation(config, market, fundamental_value, traders);

    simulation.add_agent(std::make_unique<TestAgent>());
    simulation.add_agent(std::make_unique<market::RandomTrader>(1, 100.0, 1));
    simulation.add_agent(std::make_unique<market::RandomTrader>(2, 100.0, 2));
    simulation.add_agent(std::make_unique<market::FundamentalTrader>(3, 0.02, 5));
    simulation.run();

    std::cout << "Final time: " << simulation.current_time() << '\n';
    std::cout << "Final fundamental value: " << simulation.fundamental_value() << '\n';
    std::cout << "Trades executed: " << market.trade_count() << '\n';

    return 0;
}
