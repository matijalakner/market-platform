#pragma once

#include <deque>
#include <functional>
#include <map>
#include <optional>

#include "market/core/types.hpp"
#include "market/market/order.hpp"

namespace market {
	class OrderBook {
	public:
		void add_order(const Order& order);
		bool empty() const;
		bool has_bids() const;
		bool has_asks() const;
		std::optional<Price> best_bid() const;
		std::optional<Price> best_ask() const;
		void remove_front_order(Side side);
		Order& front_order(Side side);
		void reduce_front_order(Side side, Quantity quantity);
		bool cancel_order(OrderId order_id);
	private:
		using BidBook = std::map<Price, std::deque<Order>, std::greater<Price>>;
		using AskBook = std::map<Price, std::deque<Order>, std::less<Price>>;
		BidBook bids_;
		AskBook aks_;
	};
}
