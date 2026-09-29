#include <cassert>

#include "market/market/order.hpp"

int main() {
    market::Order order{
        .id = 1,
        .trader_id = 10,
        .side = market::Side::Buy,
        .type = market::OrderType::Limit,
        .status = market::OrderStatus::New,
        .price = 100.0,
        .quantity = 100,
        .original_quantity = 100,
        .reserved_cash = 10'000.0,
        .reserved_assets = 0,
        .timestamp = 1
    };

    assert(order.is_active());
    assert(order.filled_quantity() == 0);

    order.mark_open();

    assert(order.is_active());

    order.quantity = 40;
    order.mark_partially_filled();

    assert(order.is_active());
    assert(order.filled_quantity() == 60);

    order.quantity = 0;
    order.mark_filled();
    
    assert(!order.is_active());
    assert(order.filled_quantity() == 100);

    order.quantity = 100;
    order.status = market::OrderStatus::Open;

    assert(order.is_active());

    order.mark_cancelled();

    assert(!order.is_active());
    assert(order.filled_quantity() == 0);

    return 0;
}