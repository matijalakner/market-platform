#include <cassert>

#include "market/agents/random_trader.hpp"
#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"
#include "market/market/settlement.hpp"
#include "market/models/fundamental_value.hpp"
#include "market/models/random_walk_model.hpp"

int main() {
    market::TraderRegistry traders;
    traders.add_trader(market::Trader(1, 10000.0, 100));

    market::Market market(traders);
    market::Settlement settlement(traders);

    market::RandomWalkModel model(0.0, 1.0, 1);
    market::FundamentalValue fundamental(100.0, model);

    market::RandomTrader random_trader(1, 100.0, 12345);
    random_trader.step(1, market, traders, settlement, fundamental);

    const auto& book = market.order_book();
    bool has_orders = book.has_bids() || book.has_asks();

    assert(has_orders);

    return 0;
}
