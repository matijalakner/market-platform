#include "market/agents/arbitrage_trader.hpp"

#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"
#include "market/market/settlement.hpp"
#include "market/models/fundamental_value.hpp"

namespace market {

ArbitrageTrader::ArbitrageTrader(TraderId trader_id, double threshold, Quantity order_quantity)
    : trader_id_(trader_id),
      threshold_(threshold),
      order_quantity_(order_quantity) {}

void ArbitrageTrader::step(
    Timestamp timestamp,
    Market& market,
    TraderRegistry& traders,
    Settlement& settlement,
    FundamentalValue& fundamental_value
) {
    Trader* trader = traders.find_trader(trader_id_);
    if (trader == nullptr) { return; }

    double fundamental = static_cast<double>(fundamental_value.value());
    auto ask = market.best_ask();
    auto bid = market.best_bid();

    Side side;
    Price price;
    if (ask.has_value() && static_cast<double>(ask.value()) < fundamental * (1.0 - threshold_)) {
        side = Side::Buy;
        price = ask.value();
    } else if (bid.has_value() && static_cast<double>(bid.value()) > fundamental * (1.0 + threshold_)) {
        side = Side::Sell;
        price = bid.value();
    } else {
        return;
    }

    if (side == Side::Buy) {
        if (trader->available_cash() < static_cast<double>(price) * static_cast<double>(order_quantity_)) { return; }
    } else {
        if (trader->available_assets() < order_quantity_) { return; }
    }

    Order order;
    order.trader_id = trader_id_;
    order.side = side;
    order.type = OrderType::Limit;
    order.price = price;
    order.quantity = order_quantity_;
    order.timestamp = timestamp;

    for (const Trade& trade : market.submit_order(order)) {
        settlement.settle(trade);
    }
}

}  // namespace market
