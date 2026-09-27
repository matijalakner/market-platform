#include "market/market/market.hpp"

namespace market {
    	Market::Market() 
	    	: order_book_{}, 
	    	matching_engine_(order_book_),
		order_id_generator_{} 
    	{}

	std::vector<Trade> Market::submit_order(Order order) {
		if (order.id == 0) {
			order.id = order_id_generator_.next();
		}
	    	
		std::vector<Trade> trades = matching_engine_.submit_order(order);

		for (const Trade& trade : trades) {
			trade_history_.push_back(trade);
			last_trade_price_ = trade.price;
			total_volume_ += trade.quantity;
		}

		return trades;
	}
	
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
}
