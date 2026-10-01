#include <cassert>
#include <limits>

#include "market/core/units.hpp"

int main() {
    assert(market::to_price(100.4) == 100);
    assert(market::to_price(100.5) == 101);
    assert(market::to_price(0.2) == market::kMinPrice);
    assert(market::to_price(-50.0) == market::kMinPrice);
    assert(market::to_price(std::numeric_limits<double>::quiet_NaN()) == market::kMinPrice);
    assert(market::to_price(1e30) <= 1000000000000000ULL);

    assert(market::currency_to_ticks(100.25, 0.01) == 10025);
    assert(market::currency_to_ticks(1.0, 0.5) == 2);
    assert(market::ticks_to_currency(10025, 0.01) > 100.249);
    assert(market::ticks_to_currency(10025, 0.01) < 100.251);
    return 0;
}
