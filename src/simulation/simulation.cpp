#include "market/simulation/simulation.hpp"

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <utility>

namespace market {

namespace {

// The experiment runner seeds the price model with config.seed as well. If the
// simulation's own generator (agent order, activation) started from the same
// seed, both would draw from the same underlying stream and be correlated.
// Mixing in a constant keeps runs reproducible but the streams independent.
constexpr std::uint64_t kSimulationStreamSalt = 0x9E3779B97F4A7C15ULL;

}  // namespace

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
      settlement_(traders, config.fee_rate),
      rng_(config.seed ^ kSimulationStreamSalt) {}

void Simulation::add_agent(std::unique_ptr<Agent> agent) {
    agents_.push_back(std::move(agent));
}

void Simulation::set_recorder(StatisticsRecorder* recorder) { recorder_ = recorder; }

void Simulation::schedule(Timestamp time, EventScheduler::Event callback) {
    scheduler_.schedule(time, std::move(callback));
}

void Simulation::schedule_news(Timestamp time, double relative_shock) {
    scheduler_.schedule(time, [this, relative_shock](Timestamp) {
        fundamental_value_.apply_shock(relative_shock);
    });
}

void Simulation::run() {
    for (std::size_t i = 0; i < config_.steps; ++i) {
        step();
    }
}

Timestamp Simulation::current_time() const { return current_time_; }

Price Simulation::fundamental_value() const { return fundamental_value_.value(); }

const Settlement& Simulation::settlement() const { return settlement_; }

void Simulation::step() {
    ++current_time_;

    // 1. scheduled events (news, custom callbacks)
    scheduler_.run_until(current_time_);

    // 2. the fundamental value moves
    fundamental_value_.update();

    // 3. orders whose latency has elapsed reach the book
    for (const Trade& trade : market_.process_pending(current_time_)) {
        settlement_.settle(trade);
    }

    // 4. agents act (all, or a random subset, possibly in random order)
    std::vector<std::size_t> order(agents_.size());
    std::iota(order.begin(), order.end(), 0);
    if (config_.random_order) {
        std::shuffle(order.begin(), order.end(), rng_);
    }

    std::uniform_real_distribution<double> unit(0.0, 1.0);
    for (std::size_t index : order) {
        if (config_.activation_probability < 1.0 && unit(rng_) >= config_.activation_probability) {
            continue;
        }
        agents_[index]->step(
            current_time_,
            market_,
            traders_,
            settlement_,
            fundamental_value_
        );
    }

    // 5. statistics
    if (recorder_ != nullptr) {
        recorder_->record(current_time_, market_, traders_, fundamental_value_.value());
    }
}

}  // namespace market
