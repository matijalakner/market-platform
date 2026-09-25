#include <cassert>

#include "market/agents/random_trader.hpp"
#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"

int main() {
	market::Trader trader(1, 10000.0, 100);
	market::TraderRegistry traders;
	market::Market market;
	market::RandomTrader random_trader(1, 100.0, 12345);

	traders.add_trader(trader);
	random_trader.step(1, market, traders);

	const auto& book = market.order_book();
	bool has_orders = book.has_bids() || book.has_asks();

	assert(has_orders);

	return 0;
}
	
