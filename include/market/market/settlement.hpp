#pragma once

#include "market/agents/trader_registry.hpp"
#include "market/market/trade.hpp"

namespace market {

class Settlement {
public:
    explicit Settlement(TraderRegistry& traders);

    // Moves cash and assets for a trade. Requires the buyer's cash and the
    // seller's assets to have been reserved (Market::submit_order does this).
    bool settle(const Trade& trade);

private:
    TraderRegistry& traders_;
};

}  // namespace market
