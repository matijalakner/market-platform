#pragma once

#include <cstdint>

namespace market {

using OrderId = std::uint64_t;
using TradeId = std::uint64_t;
using TraderId = std::uint64_t;
using Timestamp = std::uint64_t;

using Price = double;
using Quantity = std::uint64_t;

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
