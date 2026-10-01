#pragma once

#include "market/agents/trader_registry.hpp"
#include "market/market/trade.hpp"

namespace market {

class Settlement {
public:
    // `fee_rate` is a fraction of the trade value charged to buyer and seller.
    explicit Settlement(TraderRegistry& traders, double fee_rate = 0.0);

    // Moves cash and assets for a trade. Requires the buyer's cash and the
    // seller's assets to have been reserved (Market::submit_order does this).
    bool settle(const Trade& trade);

    double fee_rate() const;
    double total_fees() const;

private:
    TraderRegistry& traders_;
    double fee_rate_;
    double total_fees_ = 0.0;
};

}  // namespace market
