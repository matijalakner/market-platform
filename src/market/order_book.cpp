#include "market/market/order_book.hpp"

namespace market {

namespace {

template <typename Book>
bool erase_from(Book& book, OrderId order_id) {
    for (auto price_it = book.begin(); price_it != book.end(); ++price_it) {
        auto& orders = price_it->second;
        for (auto order_it = orders.begin(); order_it != orders.end(); ++order_it) {
            if (order_it->id == order_id) {
                orders.erase(order_it);
                if (orders.empty()) { book.erase(price_it); }
                return true;  // iterators are invalid now, so leave immediately
            }
        }
    }
    return false;
}

template <typename Book>
auto* find_in(Book& book, OrderId order_id) {
    for (auto& level : book) {
        for (auto& order : level.second) {
            if (order.id == order_id) { return &order; }
        }
    }
    return static_cast<decltype(&book.begin()->second.front())>(nullptr);
}

template <typename Book>
Quantity depth_of(const Book& book, std::size_t levels) {
    Quantity total = 0;
    std::size_t seen = 0;
    for (const auto& level : book) {
        if (levels != 0 && seen++ >= levels) { break; }
        for (const auto& order : level.second) { total += order.quantity; }
    }
    return total;
}

}  // namespace

Quantity OrderBook::bid_depth(std::size_t levels) const { return depth_of(bids_, levels); }
Quantity OrderBook::ask_depth(std::size_t levels) const { return depth_of(asks_, levels); }

void OrderBook::add_order(const Order& order) {
    if (order.side == Side::Buy) {
        bids_[order.price].push_back(order);
    } else {
        asks_[order.price].push_back(order);
    }
}

bool OrderBook::empty() const { return bids_.empty() && asks_.empty(); }
bool OrderBook::has_bids() const { return !bids_.empty(); }
bool OrderBook::has_asks() const { return !asks_.empty(); }

std::optional<Price> OrderBook::best_bid() const {
    if (bids_.empty()) { return std::nullopt; }
    return bids_.begin()->first;
}

std::optional<Price> OrderBook::best_ask() const {
    if (asks_.empty()) { return std::nullopt; }
    return asks_.begin()->first;
}

Order& OrderBook::front_order(Side side) {
    if (side == Side::Buy) { return bids_.begin()->second.front(); }
    return asks_.begin()->second.front();
}

void OrderBook::reduce_front_order(Side side, Quantity quantity) {
    front_order(side).quantity -= quantity;
}

void OrderBook::remove_front_order(Side side) {
    if (side == Side::Buy) {
        auto& orders = bids_.begin()->second;
        orders.pop_front();
        if (orders.empty()) { bids_.erase(bids_.begin()); }
    } else {
        auto& orders = asks_.begin()->second;
        orders.pop_front();
        if (orders.empty()) { asks_.erase(asks_.begin()); }
    }
}

bool OrderBook::cancel_order(OrderId order_id) {
    return erase_from(bids_, order_id) || erase_from(asks_, order_id);
}

Order* OrderBook::find_order(OrderId order_id) {
    if (Order* o = find_in(bids_, order_id)) { return o; }
    return find_in(asks_, order_id);
}

const Order* OrderBook::find_order(OrderId order_id) const {
    if (const Order* o = find_in(bids_, order_id)) { return o; }
    return find_in(asks_, order_id);
}

}  // namespace market
