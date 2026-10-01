#include <cassert>

#include "market/models/garch_model.hpp"
#include "market/models/geometric_brownian_model.hpp"

int main() {
    // GBM: valid and deterministic.
    market::GeometricBrownianModel g1(0.0, 0.01, 5);
    market::GeometricBrownianModel g2(0.0, 0.01, 5);
    market::Price p1 = 10000, p2 = 10000;
    for (int i = 0; i < 1000; ++i) {
        p1 = g1.next_price(p1);
        p2 = g2.next_price(p2);
        assert(p1 >= 1);
        assert(p1 == p2);
    }

    // GBM moves scale with the price level: higher price, bigger absolute moves.
    market::GeometricBrownianModel lo(0.0, 0.01, 9);
    market::GeometricBrownianModel hi(0.0, 0.01, 9);
    market::Price lo_p = lo.next_price(1000);
    market::Price hi_p = hi.next_price(100000);
    auto diff = [](market::Price a, market::Price b) { return a > b ? a - b : b - a; };
    assert(diff(hi_p, 100000) >= diff(lo_p, 1000));

    // GARCH: valid prices, volatility stays positive and finite.
    market::GarchModel garch(0.0, 1e-6, 0.1, 0.85, 3);
    market::Price p = 10000;
    for (int i = 0; i < 1000; ++i) {
        p = garch.next_price(p);
        assert(p >= 1);
        assert(garch.current_volatility() > 0.0);
        assert(garch.current_volatility() < 1.0);
    }

    return 0;
}
