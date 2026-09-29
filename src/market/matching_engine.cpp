#include "market/market/matching_engine.hpp"

#include <algorithm>
#include <utility>

namespace market {

MatchingEngine::MatchingEngine(OrderBook& order_book) : order_book_(order_book) {}

MatchResult MatchingEngine::submit_order(Order order) {
    std::vector<Trade> trades;
    if (order.side == Side::Buy) {
        trades = match_buy(order);
    } else {
        trades = match_sell(order);
    }
    finalize(order);
    return MatchResult{std::move(order), std::move(trades)};
}

std::vector<Trade> MatchingEngine::match_buy(Order& incoming) {
    std::vector<Trade> trades;

    while (incoming.quantity > 0 && order_book_.has_asks()) {
        auto best_ask = order_book_.best_ask();
        if (!best_ask.has_value()) { break; }

        // For limit orders this is the limit price; for market buys it is the
        // maximum price the trader agreed to pay (their cash is reserved at it).
        if (incoming.price < best_ask.value()) { break; }

        Order& resting = order_book_.front_order(Side::Sell);
        Quantity traded_quantity = std::min(incoming.quantity, resting.quantity);

        Trade trade;
        trade.buy_order_id = incoming.id;
        trade.sell_order_id = resting.id;
        trade.buyer_id = incoming.trader_id;
        trade.seller_id = resting.trader_id;
        trade.price = resting.price;  // trades execute at the resting order's price
        trade.quantity = traded_quantity;
        trade.timestamp = incoming.timestamp;
        trades.push_back(trade);

        incoming.quantity -= traded_quantity;

        // `resting` may be invalid after removal, so do this last.
        if (traded_quantity == resting.quantity) {
            order_book_.remove_front_order(Side::Sell);
        } else {
            order_book_.reduce_front_order(Side::Sell, traded_quantity);
        }
    }

    return trades;
}

std::vector<Trade> MatchingEngine::match_sell(Order& incoming) {
    std::vector<Trade> trades;

    while (incoming.quantity > 0 && order_book_.has_bids()) {
        auto best_bid = order_book_.best_bid();
        if (!best_bid.has_value()) { break; }

        // A limit sell only trades at or above its price.
        if (incoming.type == OrderType::Limit && incoming.price > best_bid.value()) { break; }

        Order& resting = order_book_.front_order(Side::Buy);
        Quantity traded_quantity = std::min(incoming.quantity, resting.quantity);

        Trade trade;
        trade.buy_order_id = resting.id;
        trade.sell_order_id = incoming.id;
        trade.buyer_id = resting.trader_id;
        trade.seller_id = incoming.trader_id;
        trade.price = resting.price;
        trade.quantity = traded_quantity;
        trade.timestamp = incoming.timestamp;
        trades.push_back(trade);

        incoming.quantity -= traded_quantity;

        if (traded_quantity == resting.quantity) {
            order_book_.remove_front_order(Side::Buy);
        } else {
            order_book_.reduce_front_order(Side::Buy, traded_quantity);
        }
    }

    return trades;
}

void MatchingEngine::finalize(Order& incoming) {
    if (incoming.quantity == 0) {
        incoming.status = OrderStatus::Filled;
    } else if (incoming.type == OrderType::Limit) {
        bool partially = incoming.quantity < incoming.original_quantity;
        incoming.status = partially ? OrderStatus::PartiallyFilled : OrderStatus::Open;
        order_book_.add_order(incoming);
    } else {
        // Unfilled remainder of a market order is discarded.
        incoming.status = OrderStatus::Cancelled;
    }
}

}  // namespace market
