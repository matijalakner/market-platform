#include "market/agents/fundamental_trader.hpp"

#include "market/agents/agent_utils.hpp"
#include "market/agents/trader.hpp"
#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"
#include "market/market/order.hpp"
#include "market/market/settlement.hpp"
#include "market/models/fundamental_value.hpp"

namespace market {

FundamentalTrader::FundamentalTrader(
    TraderId trader_id,
    double threshold,
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

    Price fundamental = fundamental_value.value();
    Price market_price = reference_price(market, fundamental);
    if (market_price == 0) { return; }

    double mispricing = (static_cast<double>(fundamental) - static_cast<double>(market_price))
                      / static_cast<double>(market_price);

    Side side = Side::Buy;
    if (mispricing > threshold_) {
        side = Side::Buy;    // market looks cheap
    } else if (mispricing < -threshold_) {
        side = Side::Sell;   // market looks expensive
    } else {
        return;              // hold
    }

    double order_value = static_cast<double>(market_price) * static_cast<double>(order_quantity_);
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
