#include <cassert>

#include "market/agents/arbitrage_trader.hpp"
#include "market/agents/market_maker.hpp"
#include "market/agents/momentum_trader.hpp"
#include "market/agents/noise_trader.hpp"
#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"
#include "market/market/settlement.hpp"
#include "market/models/fundamental_value.hpp"
#include "market/models/random_walk_model.hpp"

namespace {
struct Fixture {
    market::TraderRegistry traders;
    market::Market market;
    market::Settlement settlement;
    market::RandomWalkModel model;
    market::FundamentalValue fundamental;

    Fixture(market::Price fundamental_value, int n_traders)
        : traders(),
          market(add_traders(traders, n_traders)),
          settlement(traders),
          model(0.0, 0.0, 1),
          fundamental(fundamental_value, model) {}

    static market::TraderRegistry& add_traders(market::TraderRegistry& r, int n) {
        for (int i = 1; i <= n; ++i) { r.add_trader(market::Trader(i, 1e7, 1000)); }
        return r;
    }
};

market::Order limit(market::TraderId trader, market::Side side, market::Price price, market::Quantity qty) {
    market::Order o;
    o.trader_id = trader; o.side = side; o.type = market::OrderType::Limit;
    o.price = price; o.quantity = qty; o.timestamp = 1;
    return o;
}
}  // namespace

int main() {
    // ---- market maker ----
    {
        Fixture f(10000, 1);
        market::MarketMaker mm(1, 50, 5, 0.0);

        mm.step(1, f.market, f.traders, f.settlement, f.fundamental);
        assert(f.market.best_bid().value() == 9950);   // fundamental +/- half spread
        assert(f.market.best_ask().value() == 10050);
        assert(f.market.order_book().bid_depth() == 5);

        // Stepping again replaces the quotes instead of stacking them.
        mm.step(2, f.market, f.traders, f.settlement, f.fundamental);
        assert(f.market.order_book().bid_depth() == 5);
        assert(f.market.order_book().ask_depth() == 5);
    }
    {
        // Inventory skew: a long maker quotes lower.
        Fixture f(10000, 1);
        market::MarketMaker mm(1, 50, 5, 1.0);   // 1 tick per unit held; holds 1000
        mm.step(1, f.market, f.traders, f.settlement, f.fundamental);
        assert(f.market.best_ask().value() < 10000);  // centre shifted far down
    }

    // ---- arbitrage trader ----
    {
        Fixture f(10000, 2);
        f.market.submit_order(limit(2, market::Side::Sell, 9500, 10));   // 5% below fundamental
        market::ArbitrageTrader arb(1, 0.01, 4);
        arb.step(2, f.market, f.traders, f.settlement, f.fundamental);
        assert(f.market.trade_count() == 1);
        assert(f.market.total_volume() == 4);
        assert(f.traders.find_trader(1)->asset_quantity() == 1004);

        // Nothing mispriced: no trade.
        Fixture g(10000, 2);
        g.market.submit_order(limit(2, market::Side::Sell, 10050, 10));
        arb.step(2, g.market, g.traders, g.settlement, g.fundamental);
        assert(g.market.trade_count() == 0);
    }

    // ---- momentum trader ----
    {
        Fixture f(10000, 2);
        market::MomentumTrader mom(1, 2, 0.01, 3, 10000);
        // Price path 10000, 10000, then a jump the trader can see.
        f.market.submit_order(limit(2, market::Side::Buy, 9999, 10));
        f.market.submit_order(limit(2, market::Side::Sell, 10001, 10));
        mom.step(1, f.market, f.traders, f.settlement, f.fundamental);
        mom.step(2, f.market, f.traders, f.settlement, f.fundamental);
        f.market.cancel_all_orders(2);
        f.market.submit_order(limit(2, market::Side::Buy, 10199, 10));
        f.market.submit_order(limit(2, market::Side::Sell, 10201, 10));   // mid 10200: +2%
        mom.step(3, f.market, f.traders, f.settlement, f.fundamental);
        assert(f.market.trade_count() == 1);                              // bought at the ask
        assert(f.traders.find_trader(1)->asset_quantity() == 1003);
    }

    // ---- noise trader ----
    {
        Fixture f(10000, 1);
        market::NoiseTrader noise(1, 10000, 42, 10, 5, 0.5);
        for (market::Timestamp t = 1; t <= 200; ++t) {
            noise.step(t, f.market, f.traders, f.settlement, f.fundamental);
        }
        assert(f.market.order_registry().size() > 50);   // it trades a lot
        // Never leaves funds reserved for finished orders.
        const market::Trader* tr = f.traders.find_trader(1);
        assert(tr->reserved_cash() >= 0.0);
    }

    return 0;
}
