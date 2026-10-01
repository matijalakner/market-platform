#include "market/agents/noise_trader.hpp"

#include "market/agents/agent_utils.hpp"
#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"
#include "market/market/settlement.hpp"

namespace market {

NoiseTrader::NoiseTrader(
    TraderId trader_id,
    Price reference_price,
    std::uint64_t seed,
    Price max_offset,
    Quantity max_quantity,
    double market_order_probability
)
    : trader_id_(trader_id),
      reference_price_(reference_price),
      generator_(seed),
      max_offset_(max_offset),
      max_quantity_(max_quantity == 0 ? 1 : max_quantity),
      market_order_probability_(market_order_probability) {}

void NoiseTrader::step(
    Timestamp timestamp,
    Market& market,
    TraderRegistry& traders,
    Settlement& settlement,
    FundamentalValue& /*fundamental_value*/
) {
    Trader* trader = traders.find_trader(trader_id_);
    if (trader == nullptr) { return; }

    std::uniform_real_distribution<double> unit(0.0, 1.0);
    std::uniform_int_distribution<std::int64_t> offset(
        -static_cast<std::int64_t>(max_offset_), static_cast<std::int64_t>(max_offset_));
    std::uniform_int_distribution<Quantity> quantity_dist(1, max_quantity_);

    Side side = unit(generator_) < 0.5 ? Side::Buy : Side::Sell;
    Quantity quantity = quantity_dist(generator_);
    Price reference = reference_price(market, reference_price_);
    bool use_market_order = unit(generator_) < market_order_probability_;

    Order order;
    order.trader_id = trader_id_;
    order.side = side;
    order.quantity = quantity;
    order.timestamp = timestamp;

    if (use_market_order) {
        order.type = OrderType::Market;
        // For market buys, `price` caps what the trader will pay.
        order.price = side == Side::Buy ? reference + max_offset_ + 1 : 0;
    } else {
        order.type = OrderType::Limit;
        order.price = to_price(static_cast<double>(reference) + static_cast<double>(offset(generator_)));
    }

    if (side == Side::Buy) {
        if (trader->available_cash() < static_cast<double>(order.price) * static_cast<double>(quantity)) { return; }
    } else {
        if (trader->available_assets() < quantity) { return; }
    }

    for (const Trade& trade : market.submit_order(order)) {
        settlement.settle(trade);
    }
}

}  // namespace market
