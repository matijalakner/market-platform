#include "market/market/order_registry.hpp"

namespace market {
    bool OrderRegistry::add_order(const Order& order) {
        auto [it, inserted] = orders_.emplace(order.id, order);

        return inserted;
    }

    Order* OrderRegistry::find_order(OrderId order_id) {
        auto it = orders_.find(order_id);

        if (it == orders_.end()) {
            return nullptr;
        }

        return &it->second;
    }

    const Order* OrderRegistry::find_order(OrderId order_id) {
        auto it = orders_.find(order_id);

        if (it == orders_.end()) {
            return nullptr;
        }

        return &it->second;
    }

    bool OrderRegistry::update_order(const Order& order) {
        auto it = orders_.find(order.id);

        if (it == order.end()) {
            return false;
        }

        it->second = order;

        return true;
    }


    std::vector<Order> OrderRegistry::orders_for_trader(TraderId trader_id) const {
        std::vector<Order> result;

        for (const auto& [id, order] : orders_) {
            if (order.trader_id == trader_id) {
                result.push_back(order);
            }
        }

        return result;
    }

    std::size_t OrderRegistry::size() const {
        return orders_.size();
    }
}