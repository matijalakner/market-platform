#include "market/market/market.hpp"

#include <algorithm>

namespace market {

namespace {

// Gives back whatever the order still has reserved.
void release_reservation(Trader& trader, Order& order) {
    if (order.side == Side::Buy) {
        trader.release_cash(std::max(0.0, order.reserved_cash));
        order.reserved_cash = 0.0;
    } else {
        trader.release_assets(order.reserved_assets);
        order.reserved_assets = 0;
    }
}

}  // namespace

Market::Market(TraderRegistry& traders)
    : traders_(traders),
      order_book_{},
      matching_engine_(order_book_),
      order_id_generator_{},
      trade_id_generator_{} {}

const OrderBook& Market::order_book() const { return order_book_; }
const OrderRegistry& Market::order_registry() const { return order_registry_; }
const TradeHistory& Market::trade_history() const { return trade_history_; }

bool Market::has_traded() const { return !trade_history_.empty(); }
std::size_t Market::trade_count() const { return trade_history_.size(); }
Price Market::last_trade_price() const { return last_trade_price_; }
Quantity Market::total_volume() const { return total_volume_; }
double Market::total_traded_value() const { return total_traded_value_; }

std::optional<Price> Market::best_ask() const { return order_book_.best_ask(); }
std::optional<Price> Market::best_bid() const { return order_book_.best_bid(); }

std::optional<Price> Market::mid_price() const {
    auto ask = order_book_.best_ask();
    auto bid = order_book_.best_bid();
    if (!bid.has_value() || !ask.has_value()) { return std::nullopt; }
    return (bid.value() + ask.value()) / 2.0;
}

std::optional<Price> Market::spread() const {
    auto ask = order_book_.best_ask();
    auto bid = order_book_.best_bid();
    if (!bid.has_value() || !ask.has_value()) { return std::nullopt; }
    return ask.value() - bid.value();
}

std::vector<Trade> Market::submit_order(Order order) {
    Trader* trader = traders_.find_trader(order.trader_id);
    if (trader == nullptr) { return {}; }
    if (order.quantity == 0) { return {}; }
    if (order.side == Side::Buy && order.price <= 0.0) { return {}; }

    if (order.id == 0) { order.id = order_id_generator_.next(); }

    order.status = OrderStatus::New;
    order.original_quantity = order.quantity;
    order.reserved_cash = 0.0;
    order.reserved_assets = 0;

    // Reserve what the order could cost / deliver in the worst case.
    if (order.side == Side::Buy) {
        double needed = order.price * static_cast<double>(order.quantity);
        if (!trader->reserve_cash(needed)) { return {}; }
        order.reserved_cash = needed;
    } else {
        if (!trader->reserve_assets(order.quantity)) { return {}; }
        order.reserved_assets = order.quantity;
    }

    if (!order.mark_open()) {
        release_reservation(*trader, order);
        return {};
    }

    if (!order_registry_.add_order(order)) {  // e.g. duplicate order id
        release_reservation(*trader, order);
        return {};
    }

    MatchResult result = matching_engine_.submit_order(order);
    Order& incoming = result.order;

    for (auto& trade : result.trades) {
        trade.id = trade_id_generator_.next();
        trade_history_.add_trade(trade);

        last_trade_price_ = trade.price;
        total_volume_ += trade.quantity;
        total_traded_value_ += trade.value();

        // Update the resting (counterparty) order stored in the registry.
        OrderId resting_id = incoming.side == Side::Buy ? trade.sell_order_id : trade.buy_order_id;
        if (Order* resting = order_registry_.find_order(resting_id)) {
            resting->quantity -= trade.quantity;
            if (resting->side == Side::Buy) {
                resting->reserved_cash = std::max(0.0, resting->reserved_cash - trade.value());
            } else {
                resting->reserved_assets -= trade.quantity;
            }
            if (resting->quantity == 0) {
                resting->mark_filled();
            } else {
                resting->mark_partially_filled();
            }
        }

        // Bookkeeping for the incoming order's reservation.
        if (incoming.side == Side::Buy) {
            // Bought cheaper than the limit: give the difference back.
            double improvement = (incoming.price - trade.price) * static_cast<double>(trade.quantity);
            if (improvement > 0.0) { trader->release_cash(improvement); }
            incoming.reserved_cash = std::max(0.0, incoming.reserved_cash - improvement - trade.value());
        } else {
            incoming.reserved_assets -= trade.quantity;
        }
    }

    // A finished order will not rest in the book, so free what is left.
    if (!incoming.is_active()) {
        release_reservation(*trader, incoming);
    }

    order_registry_.update_order(incoming);
    return result.trades;
}

bool Market::cancel_order(OrderId order_id) {
    Order* order = order_registry_.find_order(order_id);
    if (order == nullptr || !order->is_active()) { return false; }

    Trader* trader = traders_.find_trader(order->trader_id);
    if (trader == nullptr) { return false; }

    release_reservation(*trader, *order);
    order->mark_cancelled();
    return order_book_.cancel_order(order_id);
}

const Order* Market::get_order(OrderId order_id) const {
    return order_registry_.find_order(order_id);
}

std::vector<Order> Market::orders_for_trader(TraderId trader_id) const {
    return order_registry_.orders_for_trader(trader_id);
}

double Market::vwap() const {
    if (total_volume_ == 0) { return 0.0; }
    return total_traded_value_ / static_cast<double>(total_volume_);
}

}  // namespace market
