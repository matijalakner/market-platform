#pragma once

#include "market/core/types.hpp"

namespace market {

struct Order {
    OrderId id = 0;
    TraderId trader_id = 0;

    Side side = Side::Buy;
    OrderType type = OrderType::Limit;
    OrderStatus status = OrderStatus::New;

    // For limit orders: the limit price (ticks).
    // For market buy orders: the maximum price the trader is willing to pay
    // (needed so the right amount of cash can be reserved).
    Price price = 0;

    Quantity quantity = 0;           // remaining quantity
    Quantity original_quantity = 0;

    double reserved_cash = 0.0;
    Quantity reserved_assets = 0;

    Timestamp timestamp = 0;

    bool is_active() const {
        return status == OrderStatus::New ||
               status == OrderStatus::Open ||
               status == OrderStatus::PartiallyFilled;
    }

    Quantity filled_quantity() const {
        return original_quantity - quantity;
    }

    bool mark_open() {
        if (status != OrderStatus::New) { return false; }
        status = OrderStatus::Open;
        return true;
    }

    bool mark_partially_filled() {
        if (status != OrderStatus::Open && status != OrderStatus::PartiallyFilled) { return false; }
        status = OrderStatus::PartiallyFilled;
        return true;
    }

    bool mark_filled() {
        if (status != OrderStatus::Open && status != OrderStatus::PartiallyFilled) { return false; }
        status = OrderStatus::Filled;
        return true;
    }

    bool mark_cancelled() {
        if (!is_active()) { return false; }
        status = OrderStatus::Cancelled;
        return true;
    }

    bool is_market_order() const { return type == OrderType::Market; }
    bool is_limit_order() const { return type == OrderType::Limit; }
    bool is_buy() const { return side == Side::Buy; }
    bool is_sell() const { return side == Side::Sell; }
};

}  // namespace market
