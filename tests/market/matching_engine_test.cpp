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
        .price = 100,
        .quantity = 100,
        .timestamp = 1
    });

    // Buy 1
    auto result = engine.submit_order({
        .id = 2,
        .trader_id = 20,
        .side = market::Side::Buy,
        .type = market::OrderType::Limit,
        .price = 101,
        .quantity = 50,
        .original_quantity = 50,
        .timestamp = 2
    });

    assert(result.trades.size() == 1);
    assert(result.trades[0].quantity == 50);
    assert(result.trades[0].price == 100);
    assert(result.trades[0].buyer_id == 20);
    assert(result.trades[0].seller_id == 10);
    assert(result.order.status == market::OrderStatus::Filled);

    // Buy 2
    result = engine.submit_order({
        .id = 3,
        .trader_id = 20,
        .side = market::Side::Buy,
        .type = market::OrderType::Limit,
        .price = 101,
        .quantity = 40,
        .original_quantity = 40,
        .timestamp = 3
    });

    assert(result.trades.size() == 1);
    assert(result.trades[0].quantity == 40);
    assert(book.best_ask().value() == 100);
    assert(book.find_order(1)->quantity == 10);

    // A limit buy below the best ask does not trade and rests in the book.
    result = engine.submit_order({
        .id = 4,
        .trader_id = 30,
        .side = market::Side::Buy,
        .type = market::OrderType::Limit,
        .price = 99,
        .quantity = 5,
        .original_quantity = 5,
        .timestamp = 4
    });

    assert(result.trades.empty());
    assert(result.order.status == market::OrderStatus::Open);
    assert(book.best_bid().value() == 99);

    // A limit sell above the best bid does not trade either.
    result = engine.submit_order({
        .id = 5,
        .trader_id = 40,
        .side = market::Side::Sell,
        .type = market::OrderType::Limit,
        .price = 105,
        .quantity = 5,
        .original_quantity = 5,
        .timestamp = 5
    });

    assert(result.trades.empty());

    // A limit sell at the best bid trades.
    result = engine.submit_order({
        .id = 6,
        .trader_id = 40,
        .side = market::Side::Sell,
        .type = market::OrderType::Limit,
        .price = 99,
        .quantity = 3,
        .original_quantity = 3,
        .timestamp = 6
    });

    assert(result.trades.size() == 1);
    assert(result.trades[0].price == 99);
    assert(book.find_order(4)->quantity == 2);

    return 0;
}
