#pragma once

#include "price_models.hpp"
#include "market/core/types.hpp"

namespace market {
class FundamentalValue {
    public:
        explicit FundamentalValue(Price intitial_value);
        Price value() const;
        void update();
    private:
        Price value_;
    PriceModel& model_;
};
}