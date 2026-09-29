#include "market/models/fundamental_value.hpp"

namespace market {

FundamentalValue::FundamentalValue(Price initial_value, PriceModel& model)
    : value_(initial_value),
      model_(model) {}

Price FundamentalValue::value() const { return value_; }

void FundamentalValue::update() { value_ = model_.next_price(value_); }

}  // namespace market
