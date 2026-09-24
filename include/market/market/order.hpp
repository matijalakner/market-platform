#pragma once

#include "../code/types.hpp"

namespace market {
	struct Order {
		OrderId id;
		TraderId trader_id;
		Side side;
		OrderType type;
		Price price;
		Quantity quantity;
		Timestamp timestamp;
	};
}

bool is_market_order() const;
bool is_limit_order() const;
bool is_buy() const;
bool is_sell() const;
