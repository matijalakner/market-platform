#pragma once

#include <memory>
#include <random>
#include <vector>

#include "market/agents/agent.hpp"
#include "market/agents/trader_registry.hpp"
#include "market/core/config.hpp"
#include "market/core/types.hpp"
#include "market/market/market.hpp"
#include "market/market/settlement.hpp"
#include "market/models/fundamental_value.hpp"
#include "market/simulation/event_scheduler.hpp"
#include "market/statistics/statistics.hpp"

namespace market {

class Simulation {
public:
    Simulation(
        const SimulationConfig& config,
        Market& market,
        FundamentalValue& fundamental_value,
        TraderRegistry& traders
    );

    void add_agent(std::unique_ptr<Agent> agent);

    // Optional: receives a snapshot at the end of every step.
    void set_recorder(StatisticsRecorder* recorder);

    // Runs `callback` at the start of step `time`.
    void schedule(Timestamp time, EventScheduler::Event callback);
    // Shocks the fundamental value by `relative_shock` (-0.05 = -5%) at `time`.
    void schedule_news(Timestamp time, double relative_shock);

    void run();
    void step();

    Timestamp current_time() const;
    Price fundamental_value() const;
    const Settlement& settlement() const;

private:
    SimulationConfig config_;
    Timestamp current_time_;

    Market& market_;
    FundamentalValue& fundamental_value_;
    TraderRegistry& traders_;

    Settlement settlement_;
    EventScheduler scheduler_;
    StatisticsRecorder* recorder_ = nullptr;
    std::mt19937_64 rng_;

    std::vector<std::unique_ptr<Agent>> agents_;
};

}  // namespace market
