#pragma once

#include <cstddef>
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

#include "market/agents/trader_registry.hpp"
#include "market/core/config.hpp"
#include "market/core/id_generator.hpp"
#include "market/market/matching_engine.hpp"
#include "market/market/order_book.hpp"
#include "market/market/order_registry.hpp"
#include "market/market/trade.hpp"
#include "market/market/trade_history.hpp"

namespace market {

class Market {
public:
    explicit Market(TraderRegistry& traders, const MarketConfig& config = {});

    const TradeHistory& trade_history() const;
    bool has_traded() const;
    std::size_t trade_count() const;

    Quantity total_volume() const;
    double total_traded_value() const;

    Price last_trade_price() const;
    std::optional<Price> best_ask() const;
    std::optional<Price> best_bid() const;
    // May be half a tick, hence double.
    std::optional<double> mid_price() const;
    std::optional<Price> spread() const;

    const OrderRegistry& order_registry() const;
    const OrderBook& order_book() const;

    const Order* get_order(OrderId order_id) const;
    std::vector<Order> orders_for_trader(TraderId trader_id) const;
    bool cancel_order(OrderId order_id);
    // Cancels all of a trader's active orders and returns how many there were.
    std::size_t cancel_all_orders(TraderId trader_id);

    // Reserves the trader's cash/assets, matches the order and returns the
    // resulting trades. The caller is responsible for settling them.
    //
    // With a non-zero latency the order is only queued: funds are reserved
    // now, but matching happens in process_pending() and this returns {}.
    std::vector<Trade> submit_order(Order order);

    // Matches queued orders whose arrival time has come; returns their trades.
    std::vector<Trade> process_pending(Timestamp now);
    std::size_t pending_orders() const;

    double vwap() const;

private:
    std::vector<Trade> execute(Order order);

    TraderRegistry& traders_;
    MarketConfig config_;

    OrderBook order_book_;
    MatchingEngine matching_engine_;

    IdGenerator order_id_generator_;
    IdGenerator trade_id_generator_;

    OrderRegistry order_registry_;
    TradeHistory trade_history_;

    std::multimap<Timestamp, OrderId> pending_;
    std::unordered_map<TraderId, std::vector<OrderId>> live_orders_;

    Price last_trade_price_ = 0;
    Quantity total_volume_ = 0;
    double total_traded_value_ = 0.0;
};

}  // namespace market
