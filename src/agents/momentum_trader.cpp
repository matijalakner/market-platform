#include "market/agents/momentum_trader.hpp"

#include "market/agents/agent_utils.hpp"
#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"
#include "market/market/settlement.hpp"

namespace market {

MomentumTrader::MomentumTrader(
    TraderId trader_id,
    std::size_t lookback,
    double threshold,
    Quantity order_quantity,
    Price fallback_price
)
    : trader_id_(trader_id),
      lookback_(lookback == 0 ? 1 : lookback),
      threshold_(threshold),
      order_quantity_(order_quantity),
      fallback_price_(fallback_price) {}

void MomentumTrader::step(
    Timestamp timestamp,
    Market& market,
    TraderRegistry& traders,
    Settlement& settlement,
    FundamentalValue& /*fundamental_value*/
) {
    Trader* trader = traders.find_trader(trader_id_);
    if (trader == nullptr) { return; }

    Price now = reference_price(market, fallback_price_);
    history_.push_back(now);
    if (history_.size() > lookback_ + 1) { history_.pop_front(); }
    if (history_.size() <= lookback_) { return; }

    double past = static_cast<double>(history_.front());
    double change = (static_cast<double>(now) - past) / past;

    Side side;
    Price price;
    if (change > threshold_) {
        side = Side::Buy;
        price = market.best_ask().value_or(now);
    } else if (change < -threshold_) {
        side = Side::Sell;
        price = market.best_bid().value_or(now);
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
