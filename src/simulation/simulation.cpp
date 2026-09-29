#include "market/simulation/simulation.hpp"

#include <utility>

namespace market {

Simulation::Simulation(
    const SimulationConfig& config,
    Market& market,
    FundamentalValue& fundamental_value,
    TraderRegistry& traders
)
    : config_(config),
      current_time_(0),
      market_(market),
      fundamental_value_(fundamental_value),
      traders_(traders),
      settlement_(traders) {}

void Simulation::add_agent(std::unique_ptr<Agent> agent) {
    agents_.push_back(std::move(agent));
}

void Simulation::run() {
    for (std::size_t i = 0; i < config_.steps; ++i) {
        step();
    }
}

Timestamp Simulation::current_time() const { return current_time_; }

Price Simulation::fundamental_value() const { return fundamental_value_.value(); }

void Simulation::step() {
    ++current_time_;
    fundamental_value_.update();

    for (auto& agent : agents_) {
        agent->step(
            current_time_,
            market_,
            traders_,
            settlement_,
            fundamental_value_
        );
    }
}

}  // namespace market
