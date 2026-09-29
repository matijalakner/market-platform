#pragma once

#include <vector>

#include "market/market/order.hpp"
#include "market/market/trade.hpp"

namespace market {

struct MatchResult {
    Order order;
    std::vector<Trade> trades;
};

}  // namespace market
