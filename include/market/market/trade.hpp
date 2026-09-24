#pragma once

#include "market/core/types.hpp"

namespace market {
    struct Trade {
        OrderId buy_order_id;
        OrderId sell_order_id;
        TraderId buyer_id;
        TraderId seller_id;
        Price price;
        Quantity quantity;
        Timestamp timestamp;
    };
}