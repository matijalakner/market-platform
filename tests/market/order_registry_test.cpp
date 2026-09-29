#include <cassert>

#include "market/market/order_registry.hpp"

int main() {
    market::OrderRegistry registry;

    market::Order order {
        .id = 1,
        .trader_id = 10,
        .side = market::Side::Buy,
        .type = market::OrderType::Limit,
        .status = market::OrderStatus::Open,
        .price = 100.0,
        .quantity = 100,
        .original_quantity = 100,
        .reserved_cash = 10000.0,
        .reserved_assets = 0,
        .timestamp = 1
    };

    assert(registry.add_order(order));
    assert(registry.size() == 1);

    const market::Order* found = registry.find_order(1);

    assert(found != nullptr);
    assert(found->id == 1);
    assert(found->trader_id == 10);
    assert(found->quantity == 100);

    order.quantity = 50;
    order.mark_paritally_filled();

    assert(registry.update_order(1));

    found = registry.find_order(1);

    assert(found != nullptr);
    assert(found->quantity == 50);
    assert(found->status == market::OrderStatus::PartiallyFilled);

    auto trader_orders = registry.orders_for_trader(10);

    assert(trader_orders.size() == 1);

    order.mark_filled();
    order.quantity = 0;
    registry.update_order(order);
    found = registry.find_order(1);

    assert(found->status == market::OrderStatus::Filled);
    assert(found->quantity == 0);
    assert(found->original_quantity == 100);

    return 0;
}