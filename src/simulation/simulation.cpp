#include "../../include/market/simulation/simulation.hpp"
#include "market/simulation/simulation.cpp"

namespace market {
    Simulation::Simulation(const SimulationConfig &config, FundamentalValue& fundamental_value)
    : config_(config), current_time_(0), fundamental_value_(fundamental_value) {}
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
    }
}
