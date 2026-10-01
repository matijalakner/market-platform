#pragma once

#include <cstdint>
#include <random>

#include "market/agents/agent.hpp"

namespace market {

// Trades randomly, with no view on value. Sends limit orders at a random
// offset from the reference price and, with some probability, market orders.
class NoiseTrader : public Agent {
public:
    NoiseTrader(
        TraderId trader_id,
        Price reference_price,
        std::uint64_t seed,
        Price max_offset,                 // ticks
        Quantity max_quantity,
        double market_order_probability   // 0..1
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
    Price max_offset_;
    Quantity max_quantity_;
    double market_order_probability_;
};

}  // namespace market
