#include "market/agents/fundamental_trader.hpp"

#include "market/agents/trader.hpp"
#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"
#include "market/market/order.hpp"
#include "market/market/settlement.hpp"
#include "market/models/fundamental_value.hpp"

namespace market {

FundamentalTrader::FundamentalTrader(
    TraderId trader_id,
    Price threshold,
    Quantity order_quantity
)
    : trader_id_(trader_id),
      threshold_(threshold),
      order_quantity_(order_quantity) {}

void FundamentalTrader::step(
    Timestamp timestamp,
    Market& market,
    TraderRegistry& traders,
    Settlement& settlement,
    FundamentalValue& fundamental_value
) {
    Trader* trader = traders.find_trader(trader_id_);
    if (trader == nullptr) { return; }

    Price market_price;
    auto mid = market.mid_price();

    if (mid.has_value()) {
        market_price = mid.value();
    } else if (market.has_traded()) {
        market_price = market.last_trade_price();
    } else {
        market_price = fundamental_value.value();
    }

    if (market_price <= 0.0) { return; }

    Price fundamental = fundamental_value.value();
    double mispricing = (fundamental - market_price) / market_price;

    Side side = Side::Buy;
    if (mispricing > threshold_) {
        side = Side::Buy;    // market looks cheap
    } else if (mispricing < -threshold_) {
        side = Side::Sell;   // market looks expensive
    } else {
        return;              // hold
    }

    double order_value = market_price * static_cast<double>(order_quantity_);
    if (side == Side::Buy) {
        if (trader->available_cash() < order_value) { return; }
    } else {
        if (trader->available_assets() < order_quantity_) { return; }
    }

    Order order;
    order.id = 0;
    order.trader_id = trader_id_;
    order.side = side;
    order.type = OrderType::Limit;
    order.status = OrderStatus::New;
    order.price = market_price;
    order.quantity = order_quantity_;
    order.original_quantity = order_quantity_;
    order.timestamp = timestamp;

    auto trades = market.submit_order(order);

    for (const Trade& trade : trades) {
        settlement.settle(trade);
    }
}

}  // namespace market
