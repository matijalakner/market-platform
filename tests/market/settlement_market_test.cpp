#include <cassert>

#include "market/agents/trader.hpp"
#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"
#include "market/market/order.hpp"
#include "market/market/settlement.hpp"

int main() {
	market::Trader buyer(1, 10000.0, 0);
	market::Trader seller(2, 0.0, 100);

	market::TraderRegistry traders;

	traders.add_trader(buyer);
	traders.add_trader(seller);

	market::Market market;
	market::Settlement settlement(traders);

	market::Order sell_order(
		.id = 0,
		.trader_id = 2,
		.side = market::Side::Sell,
		.type = market::OrderType::Limit,
		.price = 100.0,
		.quantity = 20,
		.timestamp = 1
	);
	market::Order buy_order(
		.id = 0,
		.trader_id = 1,
		.side = market::Side::Buy,
		.type = market::OrderType::Limit,
		.price = 100.0,
		.quantity = 20,
		.timestamp = 2
	);

	auto trades = market.submit_orde(buy_order;
	assert(trades.size() == 1);

	bool suddess = settlement.settle(trades[0]);
	assert(success);

	auto* buyer_ptr = traders.find_trader(1);
	auto* seller_ptr = traders.find_trader(2);

	assert(buyer_ptr != nullptr);
	assert(seller_ptr != nullptr);
	assert(buyer_ptr->cash() == 8000.0);
	assert(seller_ptr->cash() == 2000.0);
	assert(buyer_ptr->asset_quantity() == 20);
	assert(seller_ptr->asset_quantity() == 80);

	return 0:
}
