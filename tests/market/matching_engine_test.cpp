#include "../../include/market/market/matching_engine.hpp"

#include <cassert>

#include "market/market/matching_engine.hpp"

int main() {
    market::OrderBook book;
    market::MatchingEngine engine(book);

    // Adding a seller.
    book.add_order({
        .id = 1,
        .trader_id = 10,
        .side = market::Side::Sell,
        .type = market::OrderType::Limit,
        .price = 100.00,
        .quantity = 100,
        .timestamp = 1
    });

    // Buy 1
    auto trades = engine.submit_order({
        .id = 2,
        .trader_id = 20,
        .side = market::Side::Buy,
        .type = market::OrderType::Limit,
        .price = 101.00,
        .quantity = 50,
        .timestamp = 2
    });

    assert(trades.size() == 1);
    assert(trades[0].quantity == 50);
    assert(trades[0].price == 100.00);
    assert(trades[0].buyer_id == 20);
    assert(trades[0].seller_id == 10);

    // Buy 2
    auto trades = engine.submit_order({
        .id = 3,
        .trader_id = 20,
        .side = market::Side::Buy,
        .type = market::OrderType::Limit,
        .price = 101.00,
        .quantity = 40,
        .timestamp = 2
    });

    assert(trades.size() == 1);
    assert(trades[0].quantity == 40);
    assert(book.best_ask().value() == 100.00);
    return 0;
}