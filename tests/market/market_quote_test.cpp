#include <cassert>

#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"
#include "market/market/order.hpp"

int main() {
    market::TraderRegistry traders;
    traders.add_trader(market::Trader(1, 10000.0, 0));
    traders.add_trader(market::Trader(2, 0.0, 100));

    market::Market market(traders);

    market::Order buy_order{
        .id = 0,
        .trader_id = 1,
        .side = market::Side::Buy,
        .type = market::OrderType::Limit,
        .price = 99.0,
        .quantity = 10,
        .timestamp = 1
    };

    market::Order sell_order{
        .id = 0,
        .trader_id = 2,
        .side = market::Side::Sell,
        .type = market::OrderType::Limit,
        .price = 101.0,
        .quantity = 10,
        .timestamp = 2
    };

    market.submit_order(buy_order);
    market.submit_order(sell_order);

    assert(market.best_bid().has_value());
    assert(market.best_ask().has_value());
    assert(market.best_bid().value() == 99.0);
    assert(market.best_ask().value() == 101.0);
    assert(market.mid_price().has_value());
    assert(market.mid_price().value() == 100.0);
    assert(market.spread().value() == 2.0);

    return 0;
}
