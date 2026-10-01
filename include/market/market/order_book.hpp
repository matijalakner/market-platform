#pragma once

#include <cstddef>
#include <deque>
#include <functional>
#include <map>
#include <optional>

#include "market/core/types.hpp"
#include "market/market/order.hpp"

namespace market {

class OrderBook {
public:
    bool empty() const;
    bool has_bids() const;
    bool has_asks() const;
    std::optional<Price> best_bid() const;
    std::optional<Price> best_ask() const;

    // Total resting quantity in the best `levels` price levels (0 = all levels).
    Quantity bid_depth(std::size_t levels = 0) const;
    Quantity ask_depth(std::size_t levels = 0) const;

    void remove_front_order(Side side);
    Order& front_order(Side side);
    void reduce_front_order(Side side, Quantity quantity);

    void add_order(const Order& order);
    // Removes a resting order from the book.
    bool cancel_order(OrderId order_id);
    Order* find_order(OrderId order_id);
    const Order* find_order(OrderId order_id) const;

private:
    using BidBook = std::map<Price, std::deque<Order>, std::greater<Price>>;
    using AskBook = std::map<Price, std::deque<Order>, std::less<Price>>;

    BidBook bids_;
    AskBook asks_;
};

}  // namespace market
