#include <cassert>

#include "market/market/order.hpp"

int main() {
    market::Order order{
        .id = 1,
        .trader_id = 42,
        .side = market::Side::Buy,
        .type = market::OrderType::Limit,
        .price = 100,
        .quantity = 100,
        .timestamp = 1
    };

    assert(order.id == 1);
    assert(order.trader_id == 42);
    assert(order.side == market::Side::Buy);
    assert(order.type == market::OrderType::Limit);
    assert(order.price == 100);
    assert(order.quantity == 100);
    assert(order.timestamp == 1);
    assert(order.status == market::OrderStatus::New);
    assert(order.is_buy() && order.is_limit_order());

    return 0;
}
