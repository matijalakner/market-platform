#include "../../include/market/market/market.hpp"
#include "market/market/market.hpp"

namespace market {
    Market::Market() : order_book_{}, matching_engine_(order_book_) {}
    std::vector<Trade> Market::submit_order(Order order) { return matching_engine_.submit_order(order); }
    const OrderBook& Market::order_book() const { return order_book_; }
}
