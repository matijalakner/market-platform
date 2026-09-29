#include "market/market/trade_history.hpp"

namespace market {
    void TradeHistory::add_trade(const Trade& trade) {
        trades_.push_back(trade);
    }

    const Trade* TradeHistory::find_trade(TradeId trade_id) const {
        for (const auto& trade : trades_) {
            
            if (trade.id == trade_id) {
                return &trade;
            }
        }

        return nullptr;
    }

    const std::vector<Trade> TradeHistory::trades() const {
        return trades_;
    }

    std::vector<Trade> TradeHistory::trades_for_order(OrderId order_id) const {
        std::vector<Trade> result;

        for (const auto& trade : trades_) {

            if (trade.buy_order_id == order_id || trade.sell_order_id == order_id) {
                result.push_back(trade);
            }
        }

        return result;
    }

    std::vector<Trade>  TradeHistory::trades_for_trader(TraderId trader_id) const {
        std::vector<Trade> result;

        for (const auto& trade : trades_) {

            if (trade.buyer_id == trader_id || trade.seller_id == trader_id) {
                result.push_back(trade);
            }
        }

        return result;
    }

    std::size_t size() const {
        return trades_.size();
    }
}