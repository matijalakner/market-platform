#include <cassert>

#include "market/models/fundamental_value.hpp"
#include "market/models/random_walk_model.hpp"

int main() {
    market::RandomWalkModel model(0.0, 5.0, 12345);
    market::Price price = 10000;

    for (int i = 0; i < 1000; i++) {
        price = model.next_price(price);
        assert(price >= 1);
    }

    // Same seed => same path.
    market::RandomWalkModel a(0.1, 3.0, 7);
    market::RandomWalkModel b(0.1, 3.0, 7);
    for (int i = 0; i < 10; i++) {
        assert(a.next_price(10000) == b.next_price(10000));
    }

    // A strongly negative drift floors at one tick instead of going negative.
    market::RandomWalkModel crash(-1000.0, 0.0, 1);
    market::Price p = 100;
    for (int i = 0; i < 5; i++) { p = crash.next_price(p); }
    assert(p == 1);

    // News shocks.
    market::RandomWalkModel flat(0.0, 0.0, 1);
    market::FundamentalValue fv(1000, flat);
    fv.apply_shock(-0.10);
    assert(fv.value() == 900);
    fv.apply_shock(0.50);
    assert(fv.value() == 1350);
    fv.apply_shock(-5.0);  // absurd shock still leaves a valid price
    assert(fv.value() == 1);

    return 0;
}
