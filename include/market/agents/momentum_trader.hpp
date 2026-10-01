#pragma once

#include <cstddef>
#include <deque>

#include "market/agents/agent.hpp"

namespace market {

// Buys after the price has risen more than `threshold` (a fraction) over the
// last `lookback` steps and sells after a similar fall. Takes liquidity.
class MomentumTrader : public Agent {
public:
    MomentumTrader(
        TraderId trader_id,
        std::size_t lookback,
        double threshold,
        Quantity order_quantity,
        Price fallback_price
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
    std::size_t lookback_;
    double threshold_;
    Quantity order_quantity_;
    Price fallback_price_;
    std::deque<Price> history_;
};

}  // namespace market
