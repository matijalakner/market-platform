#pragma once

#include "market/core/types.hpp"
#include "market/models/price_model.hpp"

namespace market {

class FundamentalValue {
public:
    FundamentalValue(Price initial_value, PriceModel& model);

    Price value() const;
    void update();

private:
    Price value_;
    PriceModel& model_;
};

}  // namespace market
