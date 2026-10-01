#include <cassert>
#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "market/simulation/experiment.hpp"

int main() {
    const char* json = R"({
        "simulation": {"steps": 300, "seed": 99, "activation_probability": 0.8,
                       "random_order": true, "fee_rate": 0.0005},
        "market": {"initial_price": 50.0, "tick_size": 0.01, "latency": 1},
        "traders": {"cash": 5000, "assets": 80, "short_limit": 10},
        "model": {"type": "gbm", "mu": 0.0, "sigma": 0.002},
        "agents": {"random_traders": 5, "fundamental_traders": 3, "market_makers": 2,
                   "momentum_traders": 2, "noise_traders": 3, "arbitrage_traders": 1},
        "agent_params": {"market_maker_half_spread": 0.1, "order_quantity": 3},
        "news": [{"time": 150, "shock": -0.1}],
        "output": {"directory": "/tmp/mm_experiment_test", "trader_interval": 50}
    })";

    market::ExperimentConfig c = market::parse_experiment(json);
    assert(c.simulation.steps == 300);
    assert(c.simulation.seed == 99);
    assert(c.simulation.random_order);
    assert(c.simulation.fee_rate == 0.0005);
    assert(c.initial_price == 50.0);
    assert(c.market.latency == 1);
    assert(c.short_limit == 10);
    assert(c.model.type == "gbm");
    assert(c.agents.market_makers == 2);
    assert(c.agents.arbitrage_traders == 1);
    assert(c.params.order_quantity == 3);
    assert(c.news.size() == 1 && c.news[0].time == 150);
    assert(c.trader_interval == 50);

    market::ExperimentResult r = market::run_experiment(c);
    assert(r.summary.steps == 300);
    assert(r.trades > 0);
    assert(r.summary.first_price > 0.0);
    assert(r.total_fees > 0.0);
    assert(std::filesystem::exists("/tmp/mm_experiment_test/market.csv"));
    assert(std::filesystem::exists("/tmp/mm_experiment_test/traders.csv"));

    // Same config => identical results.
    market::ExperimentResult r2 = market::run_experiment(c);
    assert(r.trades == r2.trades);
    assert(r.summary.last_price == r2.summary.last_price);

    // Defaults apply for a nearly empty config.
    market::ExperimentConfig d = market::parse_experiment("{}");
    assert(d.simulation.steps == 1000);
    assert(d.model.type == "random_walk");

    // Bad input is rejected.
    bool threw = false;
    try { market::parse_experiment("{\"model\": {\"type\": \"nope\"}}"); }
    catch (const std::runtime_error&) { threw = true; }
    assert(threw);

    threw = false;
    try { market::parse_experiment("[1]"); } catch (const std::runtime_error&) { threw = true; }
    assert(threw);

    threw = false;
    try { market::load_experiment("/definitely/not/here.json"); }
    catch (const std::runtime_error&) { threw = true; }
    assert(threw);

    return 0;
}
