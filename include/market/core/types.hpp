#pragma once

#include <cstdint>

namespace market {

using OrderId = std::uint64_t;
using TradeId = std::uint64_t;
using TraderId = std::uint64_t;
using Timestamp = std::uint64_t;

// Prices are integers: a number of ticks. The size of one tick in currency
// units is configuration (SimulationConfig::tick_size) and only matters when
// converting to/from human-readable values.
using Price = std::uint64_t;
using Quantity = std::uint64_t;
// Signed asset holding (negative = short).
using Position = std::int64_t;

enum class Side {
    Buy,
    Sell
};

enum class OrderType {
    Market,
    Limit
};

enum class OrderStatus {
    New,
    Open,
    PartiallyFilled,
    Filled,
    Cancelled
};

}  // namespace market
