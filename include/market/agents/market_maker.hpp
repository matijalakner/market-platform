#pragma once

#include "market/agents/agent.hpp"

namespace market {

// Quotes a bid and an ask around a reference price every step, cancelling the
// previous quotes first. Quotes are shifted against its inventory
// (long -> lower quotes, short -> higher quotes) to encourage mean reversion.
class MarketMaker : public Agent {
public:
    MarketMaker(
        TraderId trader_id,
        Price half_spread,          // ticks
        Quantity quote_quantity,
        double inventory_skew       // ticks of quote shift per unit of inventory
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
    Price half_spread_;
    Quantity quote_quantity_;
    double inventory_skew_;
};

}  // namespace market
