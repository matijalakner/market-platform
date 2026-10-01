#include <cassert>

#include "market/market/trade_history.hpp"

int main() {
    market::TradeHistory history;

    market::Trade trade{
        .id = 1,
        .buy_order_id = 10,
        .sell_order_id = 20,
        .buyer_id = 100,
        .seller_id = 200,
        .price = 50,
        .quantity = 10,
        .timestamp = 123
    };

    history.add_trade(trade);

    assert(history.size() == 1);

    const market::Trade* found = history.find_trade(1);

    assert(found != nullptr);
    assert(found->price == 50);
    assert(found->quantity == 10);
    assert(found->value() == 500.0);

    auto order_trades = history.trades_for_order(10);

    assert(order_trades.size() == 1);

    auto buyer_trades = history.trades_for_trader(100);

    assert(buyer_trades.size() == 1);

    auto seller_trades = history.trades_for_trader(200);

    assert(seller_trades.size() == 1);

    return 0;
}
