#pragma once

#include <vector>

#include "matching_engine.hpp"
#include "order_book.hpp"
#include "market/market/matching_engine.hpp"
#include "market/market/trade.hpp"

namespace market {
    class Market {
    public:
        Market();
        std::vector<Trader> submit_order(Order order);
        const OrderBook& order_book() const;

    private:
        OrderBook order_book_;
        MatchingEngine matching_engine_;
    };
}