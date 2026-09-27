#include <cassert>

#include "market/market/market.hpp"
#include "market/market/order.hpp"

int main() {
	market::Market market;

	market::Order sell_order {
		.id = 0,
		.trader_id = 2,
		.side = market::Side::Sell,
		.type = market::OrderType::Limit,
		.price = 100.0,
		.quantity = 20,
		.timestamp = 1
	};
	market.submit_order(sell_order);

	market::Order buy_order {
		id = 0,
        	.trader_id = 1,
        	.side = market::Side::Buy,
        	.type = market::OrderType::Limit,
        	.price = 100.0,
        	.quantity = 20,
        	.timestamp = 2
	};

	auto trades = market.submit_order(buy_order);
	assert(traders.size() == 1);

	assert(market.trade_history().size() == 1);
	assert(market.last_trade_price() == 100.0);
	assert(market.total_volume() == 20);

	return 0;
}
