#pragma once

#include "market/core/types.hpp"

namespace market {

struct Trade {
    TradeId id = 0;

    OrderId buy_order_id = 0;
    OrderId sell_order_id = 0;

    TraderId buyer_id = 0;
    TraderId seller_id = 0;

    Price price = 0.0;
    Quantity quantity = 0;

    Timestamp timestamp = 0;

    double value() const {
        return price * static_cast<double>(quantity);
    }
};

}  // namespace market
