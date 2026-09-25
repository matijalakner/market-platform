#pragma once

#include "market/core/config.hpp"
#include "market/core/types.hpp"
#include "market/models/fundamental_value.hpp"
#include "market/market/market.hpp"

namespace market {
    class Simulation {
    public:
        explicit Simulation(
            const SimulationConfig& config,
            Market& market
            FundamentalValue& fundamental_value
        );
        void run();
        Timestamp current_time() const;
        Price fundamental_value() const;
    private:
        void step();
        SimulationConfig config_;
        Timestamp current_time_;
        Market& market_;
        FundamentalValue& fundamental_value_;
    };
}