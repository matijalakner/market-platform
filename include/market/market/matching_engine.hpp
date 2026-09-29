#pragma once

#include <vector>

#include "market/market/match_result.hpp"
#include "market/market/order_book.hpp"

namespace market {

class MatchingEngine {
public:
    explicit MatchingEngine(OrderBook& order_book);
    MatchResult submit_order(Order order);

private:
    OrderBook& order_book_;

    std::vector<Trade> match_buy(Order& incoming);
    std::vector<Trade> match_sell(Order& incoming);
    void finalize(Order& incoming);
};

}  // namespace market
