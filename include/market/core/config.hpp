#pragma once

#include <cstddef>
#include <cstdint>

#include "market/core/types.hpp"

namespace market {

struct SimulationConfig {
    std::size_t steps = 1000;
    std::uint64_t seed = 12345;

    // Probability that a given agent is activated in a given step.
    double activation_probability = 1.0;
    // Shuffle the order in which agents are activated every step.
    bool random_order = false;

    // Transaction cost, as a fraction of trade value, charged to both sides.
    double fee_rate = 0.0;
    // Currency value of one price tick (used for output and config loading).
    double tick_size = 0.01;
};

struct MarketConfig {
    // Number of steps between an order being submitted and reaching the book.
    Timestamp latency = 0;
};

}  // namespace market
