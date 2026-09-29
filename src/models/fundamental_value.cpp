#include "../../include/market/models/fundamental_value.hpp"

#include "../../include/market/agents/fundamental_trader.hpp"
#include "../../include/market/models/price_models.hpp"

namespace market {
    FundamentalValue::FundamentalValue(
        Price intial_value,
        PriceModel& model
    ) : value_(intial_value),
        model_(model) {}

    Price FundamentalValue::value() const { return value_; }
    
    void FundamentalValue::update() { value_ = model_.next_price(value_); }
}
