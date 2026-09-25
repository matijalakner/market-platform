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
	    	return matching_engine_.submit_order(order); 
	}
	
    	const OrderBook& Market::order_book() const { 
		return order_book_;
	}
}
