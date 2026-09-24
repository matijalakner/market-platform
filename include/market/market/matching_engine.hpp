#pragma once

#include <vector>

#include "order_book.hpp"
#include "trade.hpp"
#include "market/market/order_book.hpp"
#include "market/market/trade.hpp"

namespace market {
    class MatchingEngine {
    public:
        explicit MatchingEngine(OrderBook& order_book);
        std::vector<Trade> submit_order(Order order);

    private:
        OrderBook& order_book_;
        std::vector<Trade> match_buy(Order& incoming);
        std::vector<Trade> match_sell(Order& incoming);
    };
}