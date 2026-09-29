#pragma once

#include "../core/types.hpp"

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

		double reserved_cash;
		Quantity reserved_assets;

		Timestamp timestamp;

		bool is_active() const {
			return status == OrderStatus::New || status == OrderStatus::Open || status == OrderStatus::PartiallyFilled;
		}

		Quantity filled_quantity() const {
			return original_quantity- quantity;
		}
	};
}

bool is_market_order() const;
bool is_limit_order() const;
bool is_buy() const;
bool is_sell() const;
