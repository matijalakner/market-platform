#pragma once

#include <cstddef>
#include <unordered_map>
#include <vector>

#include "market/market/order.hpp"

namespace market {

class OrderRegistry {
public:
    bool add_order(const Order& order);

    Order* find_order(OrderId order_id);
    const Order* find_order(OrderId order_id) const;

    bool update_order(const Order& order);

    std::vector<Order> orders_for_trader(TraderId trader_id) const;

    std::size_t size() const;

private:
    std::unordered_map<OrderId, Order> orders_;
};

}  // namespace market
