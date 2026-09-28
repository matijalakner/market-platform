#pragma once

#include "../code/types.hpp"

namespace market {
	struct Order {
		OrderId id;
		TraderId trader_id;
		
		Side side;
		OrderType type;
		OrderStratus status;

		Price price;
		
		Quantity quantity;
		Quantity original_quantity;

		Timestamp timestamp;

		bool is_active() const {
			return status == OrderStatus::New || status == OrderStatus::Open || status == OrderStatus::PartiallyFilled;
		}
	};
}

bool is_market_order() const;
bool is_limit_order() const;
bool is_buy() const;
bool is_sell() const;
