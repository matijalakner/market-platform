#pragma once

#include "market/core/types.hpp"
#include "market/models/price_model.hpp"

namespace market {

class FundamentalValue {
public:
    FundamentalValue(Price initial_value, PriceModel& model);

    Price value() const;
    void update();

    // News: multiplies the value by (1 + relative_shock), e.g. -0.05 = -5%.
    void apply_shock(double relative_shock);

private:
    Price value_;
    PriceModel& model_;
};

}  // namespace market
