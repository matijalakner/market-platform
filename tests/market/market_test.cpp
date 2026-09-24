#include "../../include/market/market/market.hpp"

#include <cassert>

#include "market/market/market.hpp"

int main() {
    market::Market market;
    market::Order sell_order{
        .id = 1,
        .trader_id = 10,
        .side = market::Side::Sell,
        .type = market::OrderType::Limit,
        .price = 100.0,
        .quantity = 50,
        .timestamp = 1
    };

    auto trades = market.submit_order(sell_order);

    assert(trades.empty());

    auto best_ask = market.order_book().best_ask();

    assert(best_ask.has_value());
    assert(best_ask.value() == 100.0);

    market::Order buy_order {
        .id = 2,
        .trader_id = 20,
        .side = market::Side::Buy,
        .type = market::OrderType::Limit,
        .price = 100.0,
        .quantity = 20,
        .timestamp = 2
    };

    trades = market.submit_order(buy_order);

    assert(trades.size() == 1);
    assert(trades[0].price == 100.0);
    assert(trades[0].quantity == 20);

    return 0;
}