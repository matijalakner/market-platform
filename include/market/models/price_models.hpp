#pragma once

#include "market/core/types.hpp"

namespace market {
    class PriceModel {
        public:
            virtual ~PriceModel() = default;
            virtual Price next_price(Price current_price) = 0;
    };
}

