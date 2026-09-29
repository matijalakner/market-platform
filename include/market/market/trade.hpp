#pragma once

#include "market/core/types.hpp"

namespace market {
    struct Trade {
        TradeId id;

        OrderId buy_order_id;
        OrderId sell_order_id;

        TraderId buyer_id;
        TraderId seller_id;

        Price price;
        Quantity quantity;

        Timestamp timestamp;

        double value() const {
            return price * static_cast<double>(quantity);
        }
    };
}