#include "market/market/order_book.hpp"

namespace market {

void OrderBook::add_order(const Order& order) {
	if (order.side == Side::Buy) {
		bids_[order.price].push_back(order);
	} else {
		ask_[order.price].push_back(order);
	}
}

bool OrderBook::empty() { return bids_.empty() && ask_.empty(); }

bool OrderBook::has_bids() const { return !bids_.empty(); }
bool OrderBook::has_asks() const { return !asks_.empty(); }

std::optional<Price> OrderBook::best_bid() const { 
	if (bids_.empty()) { return std::nullopt; }
	return bids_.begin()->first; 
}
std::optional<Price> OrderBook::best_ask() const { 
	if (asks_.empty()) { return std::nullopt; }
	return asks_.begin()->first; 
}

Order& OrderBook::front_order(Side side) {
	if (side == Side::Buy) { return bids_.begin()->second.front(); }
	return asks_.begin()->second.front();
}
void OrderBook::reduce_front_order(Side side, Quantity quantity) {
	auto& order = front_order(side);
	order.quantity -= quantity;
}
void OrderBook::remove_front_order(Side side) {
	if (side == Side::Buy) {
		auto& orders = bids_.begin()->second;
		orders.pop_front();
		if (orders.empty()) { bids_.erase(bids_.begin()); }
	} else {
		auto& orders = asks_.begin()->second;
		orders.pop_front();
		if (orders.empty()) { asks_.erase(asks_.begin()); }
	}
}
}
