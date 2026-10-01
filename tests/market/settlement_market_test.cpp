#include <cassert>

#include "market/agents/trader.hpp"
#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"
#include "market/market/order.hpp"
#include "market/market/settlement.hpp"

int main() {
    market::Trader buyer(1, 10000.0, 0);
    market::Trader seller(2, 0.0, 100);

    market::TraderRegistry traders;

    traders.add_trader(buyer);
    traders.add_trader(seller);

    market::Market market(traders);
    market::Settlement settlement(traders);

    market::Order sell_order{
        .id = 0,
        .trader_id = 2,
        .side = market::Side::Sell,
        .type = market::OrderType::Limit,
        .price = 100,
        .quantity = 20,
        .timestamp = 1
    };
    market::Order buy_order{
        .id = 0,
        .trader_id = 1,
        .side = market::Side::Buy,
        .type = market::OrderType::Limit,
        .price = 105,  // willing to pay more than the ask: price improvement
        .quantity = 20,
        .timestamp = 2
    };

    market.submit_order(sell_order);
    auto trades = market.submit_order(buy_order);
    assert(trades.size() == 1);
    assert(trades[0].price == 100);

    bool success = settlement.settle(trades[0]);
    assert(success);

    auto* buyer_ptr = traders.find_trader(1);
    auto* seller_ptr = traders.find_trader(2);

    assert(buyer_ptr != nullptr);
    assert(seller_ptr != nullptr);
    assert(buyer_ptr->cash() == 8000.0);
    assert(seller_ptr->cash() == 2000.0);
    assert(buyer_ptr->asset_quantity() == 20);
    assert(seller_ptr->asset_quantity() == 80);

    // Nothing may stay reserved after a full fill (incl. the price improvement).
    assert(buyer_ptr->reserved_cash() == 0.0);
    assert(seller_ptr->reserved_assets() == 0);

    return 0;
}
