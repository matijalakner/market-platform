#include "market/market/market.hpp"

namespace market {
	Market::Market(TraderRegistry& traders) 
		: order_book_{}, 
		matching_engine_(order_book_),
		order_id_generator_{},
		traders_(traders)
	{}
	
	const OrderBook& Market::order_book() const { 
		return order_book_;
	}

	const std::vector<Trade>& Market::trade_history() const {
		return trade_history_;
	}
	
	bool Market::has_traded() const {
		return !trade_history_.empty();
	}

	Price Market::last_trade_price() const {
		return last_trade_price_;
	}

	bool Market::cancel_order(OrderId order_id) {
		return order_book_.cancel_order(order_id);
	}

	std::optional<Price> Market::best_ask() {
		return order_book_.best_ask();
	}

	std::optional<Price> Market::best_bid() {
		return order_book_.best_bid();
	}

	std::optional<Price> Market::mid_price() {
		auto ask = order_book_.best_ask();
		auto bid = order_book_.best_bid();

		if (!bid.has_value() || !ask.has_value()) {
			return std::nullopt;
		}

		return (bid.value() +ask.value()) / 2.0;
	}

	std::optional<Price> Market::spread() {
		auto bid = best_bid();
		auto ask = best_ask();
		
		if (!bid.has_value() || !ask.has_value()) {
			return std::nullopt;
		}

		return ask.value() - bid.value();
	}

	Quantity Market::total_volume() const {
		return total_volume_;
	}

	std::vector<Trade> Market::submit_order(Order order) {
		Trader* trader = traders_find_trader(order.trader_id);
		if (trader == nullptr) {
			return {};
		}
		
		if (order.id == 0) {
			order.id = order_id_generator_.next(); 
		}

		if (order.quantity == 0) {
			return {};
		}

		if (order.side == Side::Buy) {
			double reserved_for_remaining = order.price * static_cast<double>(order.quantity);

			double reservation_to_keep = reservation_for_remaining;
			double reservation_to_release = order.reserved_cash - reservation_to_keep;

			if (!trader->reserve_cash(reservation_to_release)) {
				return {};
			}

			order.reserved_cash = reservation_to_release;
		} else {
			if (!trader->reserve_assets(order.quantity)) {
				return {};
			}

			order.reserved_assets = order.quantity;
		}

		order.status = OrderStatus::Open;
		
		MatchResult result = matkching_engine_.submit_order(order);
		Order resulting_order = result.order;
		std::vector<Trade> trades = result.trades;

		return trades;
	}

	bool Market::cancel_order(OrderId order_id) {
		Order* order = order_book_.find_order(order_id);

		if (order == nullptr) {
			return false;
		}
		if (!order->is_active()) {
			return false;
		}

		Trader* trader = traders_.find_trader(order->trader_id);

		if (trader == nullptr) {
			return false;
		}

		if (order.side == Side::Buy) {
			trader->release_cash(order->reserved_cash);
		} else {
			trader->release_assets(order->reserved_assets);
		}

		order->reserved_cash = 0.0;
		order->reserved_assets = 0;

		order->status = OrderStatus::Cancelled;

		return order_book_.remove_order(order_id);
	}
}
