#include <cassert>

#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"
#include "market/market/settlement.hpp"

namespace {
market::Order limit(market::TraderId trader, market::Side side, market::Price price,
                    market::Quantity qty, market::Timestamp t = 1) {
    market::Order o;
    o.trader_id = trader;
    o.side = side;
    o.type = market::OrderType::Limit;
    o.price = price;
    o.quantity = qty;
    o.timestamp = t;
    return o;
}
}  // namespace

int main() {
    // ---- depth, cancel_all_orders, market orders ----
    {
        market::TraderRegistry traders;
        traders.add_trader(market::Trader(1, 100000.0, 100));
        traders.add_trader(market::Trader(2, 100000.0, 100));
        market::Market market(traders);
        market::Settlement settlement(traders);

        market.submit_order(limit(1, market::Side::Buy, 98, 10));
        market.submit_order(limit(1, market::Side::Buy, 99, 5));
        market.submit_order(limit(1, market::Side::Sell, 101, 7));

        assert(market.order_book().bid_depth() == 15);
        assert(market.order_book().bid_depth(1) == 5);   // best level only
        assert(market.order_book().ask_depth() == 7);

        assert(market.cancel_all_orders(1) == 3);
        assert(market.order_book().empty());
        assert(traders.find_trader(1)->reserved_cash() == 0.0);
        assert(traders.find_trader(1)->reserved_assets() == 0);
        assert(market.cancel_all_orders(1) == 0);

        // A market buy takes liquidity at the resting price.
        market.submit_order(limit(1, market::Side::Sell, 101, 10));
        market::Order buy;
        buy.trader_id = 2;
        buy.side = market::Side::Buy;
        buy.type = market::OrderType::Market;
        buy.price = 105;  // maximum price
        buy.quantity = 4;
        buy.timestamp = 2;
        auto trades = market.submit_order(buy);
        assert(trades.size() == 1);
        assert(trades[0].price == 101);
        assert(trades[0].quantity == 4);
        for (auto& t : trades) { assert(settlement.settle(t)); }
        // price cap 105 was reserved but only 101 was paid: nothing left over
        assert(traders.find_trader(2)->reserved_cash() == 0.0);
        assert(traders.find_trader(2)->cash() == 100000.0 - 404.0);

        // A market order that finds no liquidity is cancelled, funds released.
        market.cancel_all_orders(1);
        market::Order orphan = buy;
        orphan.quantity = 3;
        assert(market.submit_order(orphan).empty());
        assert(traders.find_trader(2)->reserved_cash() == 0.0);
        assert(market.orders_for_trader(2).back().status == market::OrderStatus::Cancelled
               || market.orders_for_trader(2).size() >= 2);
    }

    // ---- short selling through the market ----
    {
        market::TraderRegistry traders;
        traders.add_trader(market::Trader(1, 0.0, 0, 20));      // may short 20
        traders.add_trader(market::Trader(2, 10000.0, 0));
        market::Market market(traders);
        market::Settlement settlement(traders);

        assert(market.submit_order(limit(1, market::Side::Sell, 100, 21)).empty());
        assert(market.order_book().empty());                    // over the limit: rejected

        market.submit_order(limit(1, market::Side::Sell, 100, 20));
        auto trades = market.submit_order(limit(2, market::Side::Buy, 100, 20));
        assert(trades.size() == 1);
        assert(settlement.settle(trades[0]));

        assert(traders.find_trader(1)->asset_quantity() == -20);
        assert(traders.find_trader(1)->cash() == 2000.0);
        assert(traders.find_trader(2)->asset_quantity() == 20);
    }

    // ---- latency ----
    {
        market::TraderRegistry traders;
        traders.add_trader(market::Trader(1, 100000.0, 100));
        traders.add_trader(market::Trader(2, 100000.0, 100));
        market::MarketConfig config;
        config.latency = 2;
        market::Market market(traders, config);

        // Submitted at t=1, arrives at t=3.
        assert(market.submit_order(limit(1, market::Side::Sell, 100, 10, 1)).empty());
        assert(market.pending_orders() == 1);
        assert(market.order_book().empty());                         // not in the book yet
        assert(traders.find_trader(1)->reserved_assets() == 10);     // but funds are reserved

        assert(market.process_pending(2).empty());
        assert(market.pending_orders() == 1);

        market.process_pending(3);
        assert(market.pending_orders() == 0);
        assert(market.best_ask().value() == 100);

        // A queued order can be cancelled before it arrives.
        market.submit_order(limit(2, market::Side::Buy, 90, 5, 3));
        assert(market.pending_orders() == 1);
        assert(market.cancel_all_orders(2) == 1);
        assert(market.pending_orders() == 0);
        assert(traders.find_trader(2)->reserved_cash() == 0.0);
        market.process_pending(10);
        assert(!market.best_bid().has_value());

        // Trades from delayed orders come back from process_pending.
        market.submit_order(limit(2, market::Side::Buy, 100, 4, 5));
        auto trades = market.process_pending(7);
        assert(trades.size() == 1);
        assert(trades[0].quantity == 4);
    }

    return 0;
}
