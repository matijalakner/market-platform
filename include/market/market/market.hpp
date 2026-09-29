#pragma once

#include <vector>
#include <optional>

#include "market/core/id_generator.hpp"
#include "market/market/matching_engine.hpp"
#include "market/market/trade.hpp"
#include "market/market/order_registry.hpp"

namespace market {

    	class Market {
		public:
			explicit Market(TraderRegistry& traders);
			
			const TradeHistory& trade_history() const;
			bool has_traded() const;
			std::size_t trade_count() const;
			
			Quantity total_volume() const;

			double total_traded_value() const;

			Price last_trade_price() const;
			std::optional<Price> best_ask() const;
			std::optional<Price> best_bid() const;
			std::optional<Price> mid_price() const;
			std::optional<Price> spread() const;

			const OrderRegistry& order_registry() const;
			const OrderBook& order_book() const;
			
			const Order* get_order(OrderId order_id) const;
			std::vector<Order> orders_for_trader(TradeId trader_id) const;
			bool cancel_order(OrderId order_id);
			std::vector<Trader> submit_order(Order order);

			double vwap() const;

    	private:
			OrderBook order_book_;
        	MatchingEngine matching_engine_;

			IdGenerator oder_id_generator_;
			IdGenerator trade_id_generator_;

			TradeHistory trade_history_;
			Price last_trade_price_ = 0.0;
			Quantity total_volume_ = 0;

			TraderRegistry& traders_;
			OrderRegistry order_registry_;

			double total_traded_value_ = 0.0;
    	};
}
