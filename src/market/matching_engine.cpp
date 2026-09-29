#include <algorithm>

#include "include/market/market/matching_engine.hpp"

namespace market {
	MatchingEngine::MatchingEngine(OrderBook &order_book) : order_book_(order_book) {}

	MatchResult MatchingEngine::submit_order(Order order) {
		std::vector<Trade> trades;
		if (order.side == Side::Buy) { 
			trades = match_buy(order); 
		} else {
			trades = match_sell(order);
		}
		return MatchResult{
			.order = order,
			.trades = trades
		};}

	std::vector<Trade> MatchingEngine::match_buy(Order& incoming) {
		std::vector<Trade> trades;

		while (incoming.quantity > 0 && order_book_.has_asks()) {
			auto best_ask = order_book_.best_ask();

			if (!best_ask.has_value()) { break; }
			if (incoming.type == OrderType::Limit && incoming.price < best_ask.value()) { break; }

			Order& resting = order_book_.front_order(Side::Sell);

			Quantity traded_quantity = std::min(incoming.quantity, resting.quantity);
			resting.quantity -= traded_quantity;
		
			if (resting.quantity == 0) {
				resting.status = OrderStatus::Filled;
				order_book_.remove_front_order(Side::Sell);
			} else {
				resting.status = OrderStatus::PartiallyFilled;
			}

			Trade trade {
				.buy_order_id = incoming.id,
				.sell_order_id = resting.id,
				.buyer_id = incoming.trader_id,
				.seller_id = resting.trader_id,
				.price = resting.price,
				.quantity = traded_quantity,
				.timestamp = incoming.timestamp
			};

			trades.push_back(trade);
			incoming.quantity -= traded_quantity;

			if (incoming.quantity == 0) {
				incoming.status = OrderStatus::Filled;
			} else {
				incoming.status = OrderStatus::PartiallyFilled;
			}

			if (traded_quantity == resting.quantity) { 
				order_book_.remove_front_order(Side::Sell); 
			} else { 
				order_book_.reduce_front_order(Side::Sell, traded_quantity); 
			}
		}

		if (incoming.quantity > 0 && incoming.type == OrderType::Limit) { 
			incoming.status = OrderStatus::Open;
			order_book_.add_order(incoming); 
		} else if (incoming.quantity == 0) {
			incoming.status = OrderStatus::Filled;
		}

		return trades;}

    std::vector<Trade> MatchingEngine::match_sell(Order& incoming) {
		std::vector<Trade> trades;

		while (incoming.quantity > 0 && order_book_.has_bids()) {
			auto best_bid = order_book_.best_bid();

			if (!best_bid.has_value()) { break; }
			if (incoming.type == OrderType::Limit && incoming.price < best_bid.value()) { break; }

			Order& resting = order_book_.front_order(Side::Buy);
			Quantity traded_quantity = std::min(incoming.quantity, resting.quantity);

			Trade trade {
				.buy_order_id = resting.id,
				.sell_order_id = incoming.id,
				.buyer_id = resting.trader_id,
				.seller_id = incoming.trader_id,
				.price = resting.price,
				.quantity = traded_quantity,
				.timestamp = incoming.timestamp
			};

			trades.push_back(trade);
			incoming.quantity -= traded_quantity;

			if (traded_quantity == resting.quantity) { 
				order_book_.remove_front_order(Side::Buy); 
			} else { 
				order_book_.reduce_front_order(Side::Buy, traded_quantity); 
			}
		}
	
		if (incoming.quantity > 0 && incoming.type == OrderType::Limit) { 
			incoming.status = OrderStatus::Open;
			order_book_.add_order(incoming); 
		} else if (incoming.quantity == 0) {
			incoming.status = OrderStatus::Filled;
		}
	
		return trades;}
}
