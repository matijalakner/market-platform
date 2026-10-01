#include <cassert>

#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"

int main() {
    market::TraderRegistry traders;
    traders.add_trader(market::Trader(10, 0.0, 100));
    traders.add_trader(market::Trader(20, 10000.0, 0));

    market::Market market(traders);

    market::Order sell_order{
        .id = 1,
        .trader_id = 10,
        .side = market::Side::Sell,
        .type = market::OrderType::Limit,
        .price = 100,
        .quantity = 50,
        .timestamp = 1
    };

    auto trades = market.submit_order(sell_order);

    assert(trades.empty());
    assert(traders.find_trader(10)->reserved_assets() == 50);

    auto best_ask = market.order_book().best_ask();

    assert(best_ask.has_value());
    assert(best_ask.value() == 100.0);

    market::Order buy_order{
        .id = 2,
        .trader_id = 20,
        .side = market::Side::Buy,
        .type = market::OrderType::Limit,
        .price = 100,
        .quantity = 20,
        .timestamp = 2
    };

    trades = market.submit_order(buy_order);

    assert(trades.size() == 1);
    assert(trades[0].price == 100);
    assert(trades[0].quantity == 20);

    const market::Order* resting = market.get_order(1);
    assert(resting != nullptr);
    assert(resting->quantity == 30);
    assert(resting->status == market::OrderStatus::PartiallyFilled);
    assert(market.get_order(2)->status == market::OrderStatus::Filled);

    // Cancelling the rest of the sell order releases the reserved assets
    // (20 of the 50 are still reserved for the unsettled trade).
    assert(market.cancel_order(1));
    assert(traders.find_trader(10)->reserved_assets() == 20);
    assert(!market.order_book().best_ask().has_value());
    assert(!market.cancel_order(1));

    // A trader without enough cash is rejected.
    market::Order too_big{
        .id = 0,
        .trader_id = 20,
        .side = market::Side::Buy,
        .type = market::OrderType::Limit,
        .price = 1000,
        .quantity = 1000,
        .timestamp = 3
    };
    assert(market.submit_order(too_big).empty());

    return 0;
}
