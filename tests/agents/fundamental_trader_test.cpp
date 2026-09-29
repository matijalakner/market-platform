#include <cassert>

#include "market/agents/fundamental_trader.hpp"
#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"
#include "market/market/settlement.hpp"
#include "market/models/fundamental_value.hpp"
#include "market/models/price_model.hpp"

class ConstantPriceModel : public market::PriceModel {
public:
    market::Price next_price(market::Price current_price) override {
        return current_price;
    }
};

int main() {
    ConstantPriceModel model;
    market::FundamentalValue fundamental(110.0, model);

    market::TraderRegistry traders;
    traders.add_trader(market::Trader(1, 10000.0, 100));  // fundamental trader
    traders.add_trader(market::Trader(2, 10000.0, 100));  // quote provider

    market::Market market(traders);
    market::Settlement settlement(traders);

    // Trader 2 quotes 99 / 100, so the mid price is 99.5.
    market.submit_order({
        .id = 0, .trader_id = 2, .side = market::Side::Buy,
        .type = market::OrderType::Limit, .price = 99.0,
        .quantity = 10, .timestamp = 1
    });
    market.submit_order({
        .id = 0, .trader_id = 2, .side = market::Side::Sell,
        .type = market::OrderType::Limit, .price = 100.0,
        .quantity = 10, .timestamp = 1
    });

    // Fundamental value (110) is well above the mid price (99.5), so the
    // trader buys. The buy at the mid price does not cross the ask of 100
    // and should now be resting in the book as the best bid.
    market::FundamentalTrader fundamental_trader(1, 0.02, 10);
    fundamental_trader.step(2, market, traders, settlement, fundamental);

    assert(market.best_bid().has_value());
    assert(market.best_bid().value() == 99.5);
    assert(market.orders_for_trader(1).size() == 1);

    // If the fundamental value is close to the market price, it holds.
    market::FundamentalValue fair(99.6, model);
    market::FundamentalTrader holder(2, 0.02, 10);
    holder.step(3, market, traders, settlement, fair);
    assert(market.orders_for_trader(2).size() == 2);  // no new order

    return 0;
}
