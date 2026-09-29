#include <cassert>

#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"
#include "market/market/order.hpp"

int main() {
    market::TraderRegistry traders;
    traders.add_trader(market::Trader(1, 10000.0, 0));
    traders.add_trader(market::Trader(2, 0.0, 100));

    market::Market market(traders);

    market::Order sell_order{
        .id = 0,
        .trader_id = 2,
        .side = market::Side::Sell,
        .type = market::OrderType::Limit,
        .price = 100.0,
        .quantity = 20,
        .timestamp = 1
    };
    market.submit_order(sell_order);

    market::Order buy_order{
        .id = 0,
        .trader_id = 1,
        .side = market::Side::Buy,
        .type = market::OrderType::Limit,
        .price = 100.0,
        .quantity = 20,
        .timestamp = 2
    };

    auto trades = market.submit_order(buy_order);
    assert(trades.size() == 1);

    assert(market.trade_history().size() == 1);
    assert(market.last_trade_price() == 100.0);
    assert(market.total_volume() == 20);
    assert(market.vwap() == 100.0);

    return 0;
}
