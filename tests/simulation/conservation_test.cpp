#include <cassert>
#include <cmath>
#include <memory>

#include "market/agents/arbitrage_trader.hpp"
#include "market/agents/fundamental_trader.hpp"
#include "market/agents/market_maker.hpp"
#include "market/agents/momentum_trader.hpp"
#include "market/agents/noise_trader.hpp"
#include "market/agents/random_trader.hpp"
#include "market/models/garch_model.hpp"
#include "market/simulation/simulation.hpp"

// Runs a busy simulation (all agent types, fees, latency, short selling,
// news) and checks the books balance: money and assets are neither created
// nor destroyed, and reservations always match the orders still alive.
int main() {
    const double initial_cash = 1e6;
    const market::Position initial_assets = 100;
    const int n = 8;

    market::TraderRegistry traders;
    for (int i = 1; i <= n; ++i) {
        traders.add_trader(market::Trader(i, initial_cash, initial_assets, 30));
    }

    market::MarketConfig mc;
    mc.latency = 1;
    market::Market market(traders, mc);

    market::GarchModel model(0.0, 1e-6, 0.1, 0.85, 5);
    market::FundamentalValue fundamental(10000, model);

    market::SimulationConfig config;
    config.steps = 3000;
    config.seed = 11;
    config.fee_rate = 0.001;
    config.random_order = true;
    config.activation_probability = 0.8;

    market::Simulation sim(config, market, fundamental, traders);
    sim.add_agent(std::make_unique<market::MarketMaker>(1, 20, 5, 0.5));
    sim.add_agent(std::make_unique<market::MarketMaker>(2, 30, 5, 0.5));
    sim.add_agent(std::make_unique<market::RandomTrader>(3, 10000, 3));
    sim.add_agent(std::make_unique<market::FundamentalTrader>(4, 0.01, 4));
    sim.add_agent(std::make_unique<market::MomentumTrader>(5, 5, 0.002, 4, 10000));
    sim.add_agent(std::make_unique<market::NoiseTrader>(6, 10000, 6, 30, 5, 0.3));
    sim.add_agent(std::make_unique<market::NoiseTrader>(7, 10000, 7, 30, 5, 0.3));
    sim.add_agent(std::make_unique<market::ArbitrageTrader>(8, 0.005, 4));
    sim.schedule_news(1000, -0.05);
    sim.schedule_news(2000, 0.04);
    sim.run();

    assert(market.trade_count() > 100);

    double cash = 0.0, reserved_cash = 0.0;
    market::Position assets = 0;
    std::int64_t reserved_assets = 0;
    for (int i = 1; i <= n; ++i) {
        const market::Trader* t = traders.find_trader(i);
        cash += t->cash();
        reserved_cash += t->reserved_cash();
        assets += t->asset_quantity();
        reserved_assets += static_cast<std::int64_t>(t->reserved_assets());
        assert(t->cash() >= -1e-6);
        assert(t->reserved_cash() >= 0.0);
        assert(t->asset_quantity() >= -static_cast<market::Position>(t->short_limit()));
    }

    // Money: only fees leave the system. Assets: nothing is created.
    assert(std::fabs((cash + sim.settlement().total_fees()) - n * initial_cash) < 1e-3);
    assert(assets == n * initial_assets);

    // Reservations must match the live orders exactly.
    double live_cash = 0.0;
    std::int64_t live_assets = 0;
    for (int i = 1; i <= n; ++i) {
        for (const market::Order& o : market.orders_for_trader(i)) {
            if (!o.is_active()) {
                assert(o.reserved_cash == 0.0);
                assert(o.reserved_assets == 0);
            } else {
                live_cash += o.reserved_cash;
                live_assets += static_cast<std::int64_t>(o.reserved_assets);
            }
        }
    }
    assert(std::fabs(live_cash - reserved_cash) < 1e-3);
    assert(live_assets == reserved_assets);

    return 0;
}
