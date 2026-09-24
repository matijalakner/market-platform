#include <cassert>

#include "include/market/market/order_book.hpp"

int main() {
	market::OrderBook book;

	assert(book.empty());
	assert(!book.has_bids());
	assert(!book.has_asks());

	book.add_order({
		.id = 1,
		.trader_id = 10,
		.side = market::Side::Buy,
		.type = market::OrderType::Limit,
		.price = 99.50,
		.quantity = 100,
		.timestamp = 1
	});

	book.add_order({
		.id = 2,
		.trader_id = 20,
		.side = market::Side::Buy,
		.type = market::OrderType::Limit,
		.price = 100.00,
		.quantity = 50,
		.timestamp = 2
	});

	book.add_order({
		.id = 3,
		.trader_id = 30,
		.side = market::Side::Sell,
		.type = market::OrderType::Limit,
		.price = 101.00,
		.quantity = 75,
		.timestamp = 3
	});

	assert(book.has_bids());
	assert(book.has_asks());

	assert(book.best_bid().value() == 100.00);
	assert(book.best_ask().value() == 101.00);

	return 0;
}