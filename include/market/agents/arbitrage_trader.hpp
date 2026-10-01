#pragma once

#include "market/agents/agent.hpp"

namespace market {

// Takes liquidity when a quote is clearly away from the fundamental value:
// buys the best ask if it is more than `threshold` (a fraction) below it,
// sells into the best bid if it is more than `threshold` above it.
class ArbitrageTrader : public Agent {
public:
    ArbitrageTrader(
        TraderId trader_id,
        double threshold,
        Quantity order_quantity
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
    double threshold_;
    Quantity order_quantity_;
};

}  // namespace market
