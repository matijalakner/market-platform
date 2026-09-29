#include <iostream>

#include "market/core/types.hpp"

int main() {
	market::OrderId order_id = 1;
	market::TraderId trader_id = 42;

	market::Price price = 100.50;
	market::Quantity quantity = 10;

	market::Side side = market::Side::Buy;
	market::OrderType type = market::OrderType::Limit;

	std::cout << "Order ID: " << order_id << '\n';
	std::cout << "Trader ID: " << trader_id << '\n';
	std::cout << "Price: " << price << '\n';
	std::cout << "Quantity: " << quantity << '\n';
	
	return 0;
}
