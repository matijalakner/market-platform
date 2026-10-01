#include "market/agents/market_maker.hpp"

#include "market/agents/agent_utils.hpp"
#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"
#include "market/market/settlement.hpp"
#include "market/models/fundamental_value.hpp"

namespace market {

MarketMaker::MarketMaker(
    TraderId trader_id,
    Price half_spread,
    Quantity quote_quantity,
    double inventory_skew
)
    : trader_id_(trader_id),
      half_spread_(half_spread == 0 ? 1 : half_spread),
      quote_quantity_(quote_quantity),
      inventory_skew_(inventory_skew) {}

void MarketMaker::step(
    Timestamp timestamp,
    Market& market,
    TraderRegistry& traders,
    Settlement& settlement,
    FundamentalValue& fundamental_value
) {
    Trader* trader = traders.find_trader(trader_id_);
    if (trader == nullptr) { return; }

    market.cancel_all_orders(trader_id_);

    Price reference = reference_price(market, fundamental_value.value());
    double center = static_cast<double>(reference)
                  - inventory_skew_ * static_cast<double>(trader->asset_quantity());

    Price bid_price = to_price(center - static_cast<double>(half_spread_));
    Price ask_price = to_price(center + static_cast<double>(half_spread_));
    if (ask_price <= bid_price) { ask_price = bid_price + 1; }

    for (Side side : {Side::Buy, Side::Sell}) {
        Order order;
        order.trader_id = trader_id_;
        order.side = side;
        order.type = OrderType::Limit;
        order.price = side == Side::Buy ? bid_price : ask_price;
        order.quantity = quote_quantity_;
        order.timestamp = timestamp;

        // The market rejects the quote if the trader can't fund it.
        for (const Trade& trade : market.submit_order(order)) {
            settlement.settle(trade);
        }
    }
}

}  // namespace market
