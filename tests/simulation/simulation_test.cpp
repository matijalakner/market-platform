#include <cassert>
#include <memory>

#include "market/agents/random_trader.hpp"
#include "market/models/random_walk_model.hpp"
#include "market/simulation/simulation.hpp"

namespace {

class CountingAgent : public market::Agent {
public:
    explicit CountingAgent(int* counter) : counter_(counter) {}
    void step(market::Timestamp, market::Market&, market::TraderRegistry&,
              market::Settlement&, market::FundamentalValue&) override { ++*counter_; }
private:
    int* counter_;
};

class OrderAgent : public market::Agent {
public:
    OrderAgent(market::TraderId id, market::Side side) : id_(id), side_(side) {}
    void step(market::Timestamp t, market::Market& m, market::TraderRegistry&,
              market::Settlement& s, market::FundamentalValue&) override {
        market::Order o;
        o.trader_id = id_; o.side = side_; o.type = market::OrderType::Limit;
        o.price = 100; o.quantity = 1; o.timestamp = t;
        for (const auto& trade : m.submit_order(o)) { s.settle(trade); }
    }
private:
    market::TraderId id_;
    market::Side side_;
};

}  // namespace

int main() {
    // ---- activation ----
    {
        market::TraderRegistry traders;
        market::Market market(traders);
        market::RandomWalkModel model(0.0, 0.0, 1);
        market::FundamentalValue fundamental(10000, model);

        market::SimulationConfig config;
        config.steps = 100;

        int count = 0;
        market::Simulation sim(config, market, fundamental, traders);
        sim.add_agent(std::make_unique<CountingAgent>(&count));
        sim.run();
        assert(count == 100);
        assert(sim.current_time() == 100);

        // Random activation: roughly half the steps.
        config.activation_probability = 0.5;
        int half = 0;
        market::Simulation sim2(config, market, fundamental, traders);
        sim2.add_agent(std::make_unique<CountingAgent>(&half));
        sim2.run();
        assert(half > 25 && half < 75);

        // Probability 0: agents never act.
        config.activation_probability = 0.0;
        int none = 0;
        market::Simulation sim3(config, market, fundamental, traders);
        sim3.add_agent(std::make_unique<CountingAgent>(&none));
        sim3.run();
        assert(none == 0);
    }

    // ---- news ----
    {
        market::TraderRegistry traders;
        market::Market market(traders);
        market::RandomWalkModel model(0.0, 0.0, 1);
        market::FundamentalValue fundamental(10000, model);

        market::SimulationConfig config;
        config.steps = 10;
        market::Simulation sim(config, market, fundamental, traders);
        sim.schedule_news(5, -0.10);

        bool called = false;
        sim.schedule(3, [&](market::Timestamp t) { called = true; assert(t == 3); });

        sim.run();
        assert(called);
        assert(sim.fundamental_value() == 9000);
    }

    // ---- latency + fees flow through the simulation ----
    {
        market::TraderRegistry traders;
        traders.add_trader(market::Trader(1, 100000.0, 0));
        traders.add_trader(market::Trader(2, 0.0, 100));
        market::MarketConfig mc;
        mc.latency = 1;
        market::Market market(traders, mc);
        market::RandomWalkModel model(0.0, 0.0, 1);
        market::FundamentalValue fundamental(100, model);

        market::SimulationConfig config;
        config.steps = 1;
        config.fee_rate = 0.01;
        market::Simulation sim(config, market, fundamental, traders);

        // Sell and buy both submitted at t=1 arrive at t=2.
        sim.add_agent(std::make_unique<OrderAgent>(2, market::Side::Sell));
        sim.add_agent(std::make_unique<OrderAgent>(1, market::Side::Buy));
        sim.step();
        assert(market.trade_count() == 0);
        assert(market.pending_orders() == 2);
        sim.step();   // orders arrive and match; the simulation settles them
        assert(market.trade_count() == 1);
        assert(traders.find_trader(1)->asset_quantity() == 1);
        assert(sim.settlement().total_fees() > 0.0);
    }

    // ---- reproducibility ----
    {
        auto run = [](std::uint64_t seed) {
            market::TraderRegistry traders;
            for (int i = 1; i <= 4; ++i) { traders.add_trader(market::Trader(i, 1e6, 100)); }
            market::Market market(traders);
            market::RandomWalkModel model(0.0, 3.0, seed);
            market::FundamentalValue fundamental(10000, model);
            market::SimulationConfig config;
            config.steps = 200;
            config.seed = seed;
            config.random_order = true;
            config.activation_probability = 0.7;
            market::Simulation sim(config, market, fundamental, traders);
            for (int i = 1; i <= 4; ++i) {
                sim.add_agent(std::make_unique<market::RandomTrader>(i, 10000, seed + i));
            }
            sim.run();
            return std::make_pair(market.trade_count(), market.total_volume());
        };
        assert(run(7) == run(7));
    }

    return 0;
}
