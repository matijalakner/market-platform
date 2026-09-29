#pragma once

#include <cstdint>
#include <random>

#include "market/agents/agent.hpp"

namespace market {

class RandomTrader : public Agent {
public:
    RandomTrader(
        TraderId trader_id,
        Price reference_price,
        std::uint64_t seed
    );

    void step(
        Timestamp timestamp,
        Market& market,
        TraderRegistry& traders,
        Settlement& settlement,
        FundamentalValue& fundamental_value
    ) override;

private:
    TraderId trader_id_;
    Price reference_price_;
    std::mt19937_64 generator_;
    std::uniform_int_distribution<int> side_distribution_;
    std::uniform_int_distribution<int> quantity_distribution_;
};

}  // namespace market
