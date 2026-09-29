#pragma once

#include <vector>
#include <optional>

#include "market/core/id_generator.hpp"
#include "market/market/matching_engine.hpp"
#include "market/market/trade.hpp"

namespace market {

    	class Market {
		public:
			explicit Market(TraderRegistry& traders);
			std::vector<Trader> submit_order(Order order);
			const OrderBook& order_book() const;
			const std::vector<Trade>& trade_history() const;
			bool has_traded() const;
			Price last_trade_price() const;
			Quantity total_volume() const;
			bool cancel_order(OrderId order_id);
			std::vector<Trade> submit_order(Order order);

			std::optional<Price> best_ask() const;
			std::optional<Price> best_bid() const;
			std::optional<Price> mid_price() const;
			std::optional<Price> spread() const;

    		private:
			OrderBook order_book_;
        		MatchingEngine matching_engine_;
			IdGenerator oder_id_generator_;

			std::vector<Trade> trader_history_;
			Price last_trade_price_ = 0.0;
			Quantity total_volume_ = 0;

			TraderRegistry& traders_;
    	};
}
