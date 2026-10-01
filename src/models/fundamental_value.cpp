#include "market/models/fundamental_value.hpp"

#include "market/core/units.hpp"

namespace market {

FundamentalValue::FundamentalValue(Price initial_value, PriceModel& model)
    : value_(initial_value == 0 ? kMinPrice : initial_value),
      model_(model) {}

Price FundamentalValue::value() const { return value_; }

void FundamentalValue::update() { value_ = model_.next_price(value_); }

void FundamentalValue::apply_shock(double relative_shock) {
    value_ = to_price(static_cast<double>(value_) * (1.0 + relative_shock));
}

}  // namespace market
