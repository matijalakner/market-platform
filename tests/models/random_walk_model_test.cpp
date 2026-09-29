#include <cassert>
#include <cmath>

#include "market/models/random_walk_model.hpp"

int main() {
    market::RandomWalkModel model(0.0, 1.0, 12345);
    double price = 100.00;

    for (int i = 0; i < 1000; i++) {
        price = model.next_price(price);
        assert(std::isfinite(price));
    }

    // Same seed => same path.
    market::RandomWalkModel a(0.1, 1.0, 7);
    market::RandomWalkModel b(0.1, 1.0, 7);
    for (int i = 0; i < 10; i++) {
        assert(a.next_price(100.0) == b.next_price(100.0));
    }

    return 0;
}
