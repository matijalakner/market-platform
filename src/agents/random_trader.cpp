#include "market/agents/random_trader.hpp"

#include "market/agents/agent_utils.hpp"
#include "market/agents/trader.hpp"
#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"
#include "market/market/order.hpp"
#include "market/market/settlement.hpp"

namespace market {

RandomTrader::RandomTrader(
    TraderId trader_id,
    Price reference_price,
    std::uint64_t seed
)
    : trader_id_(trader_id),
      reference_price_(reference_price),
      generator_(seed),
      side_distribution_(0, 1),
      quantity_distribution_(1, 10) {}

void RandomTrader::step(
    Timestamp timestamp,
    Market& market,
    TraderRegistry& traders,
    Settlement& settlement,
    FundamentalValue& /*fundamental_value*/
) {
    Trader* trader = traders.find_trader(trader_id_);
    if (trader == nullptr) { return; }

    int side_value = side_distribution_(generator_);
    Quantity quantity = static_cast<Quantity>(quantity_distribution_(generator_));
    Side side = side_value == 0 ? Side::Buy : Side::Sell;

    Price price = reference_price(market, reference_price_);

    // Market::submit_order reserves cash/assets itself (and rejects the
    // order if the trader can't afford it), so don't reserve here too.
    double order_value = static_cast<double>(price) * static_cast<double>(quantity);
    if (side == Side::Buy) {
        if (trader->available_cash() < order_value) { return; }
    } else {
        if (trader->available_assets() < quantity) { return; }
    }

    Order order;
    order.id = 0;
    order.trader_id = trader->id();
    order.side = side;
    order.type = OrderType::Limit;
    order.price = price;
    order.quantity = quantity;
    order.original_quantity = quantity;
    order.timestamp = timestamp;

    auto trades = market.submit_order(order);

    for (const Trade& trade : trades) {
        settlement.settle(trade);
    }
}

}  // namespace market
