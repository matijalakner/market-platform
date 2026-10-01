#pragma once

#include "market/core/units.hpp"
#include "market/market/market.hpp"

namespace market {

// Best available estimate of the current market price, in ticks:
// mid price, else last trade price, else `fallback`.
inline Price reference_price(const Market& market, Price fallback) {
    if (auto mid = market.mid_price()) { return to_price(*mid); }
    if (market.has_traded()) { return market.last_trade_price(); }
    return fallback;
}

}  // namespace market
