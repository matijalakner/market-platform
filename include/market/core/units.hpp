#pragma once

#include <cmath>

#include "market/core/types.hpp"

namespace market {

// The lowest valid price. Prices never reach zero or go negative.
constexpr Price kMinPrice = 1;

// Rounds a (possibly fractional) tick value to a valid Price.
// NaN, negative and sub-tick values are clamped to kMinPrice.
inline Price to_price(double ticks) {
    if (!(ticks >= 1.0)) { return kMinPrice; }
    if (ticks > 1e15) { ticks = 1e15; }
    return static_cast<Price>(std::llround(ticks));
}

// Converts a currency amount (e.g. 100.25) to ticks.
inline Price currency_to_ticks(double value, double tick_size) {
    return to_price(value / tick_size);
}

// Money amounts (cash, P&L) are stored in "tick units": price in ticks
// times quantity. This converts them back to currency.
inline double ticks_to_currency(double ticks, double tick_size) {
    return ticks * tick_size;
}

}  // namespace market
