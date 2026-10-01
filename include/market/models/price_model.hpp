#pragma once

#include "market/core/types.hpp"

namespace market {

// Produces the next fundamental value (in ticks) from the current one.
// Implementations must return a valid price (>= 1).
class PriceModel {
public:
    virtual ~PriceModel() = default;
    virtual Price next_price(Price current_price) = 0;
};

}  // namespace market
