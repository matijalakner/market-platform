#pragma once

#include <vector>

#include "market/market/trade.hpp"

namespace market {
    class TradeHistory {
        public:
            void add_trade(const Trade& trade);
            const Trade* find_trade(TradeId trade_id) const;

            const std::vector<Trade>& trades() const;

            std::vector<Trade> trades_for_order(OrderId order_id) const;
            std::vector<Trade> trades_for_trader(TraderId trader_id) const;

            std::size_t size() const;

        private:
            std::vector<Trade> trades_;
    }
}