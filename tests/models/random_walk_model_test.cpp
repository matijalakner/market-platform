#include "../../include/market/models/random_walk_model.hpp"

#include <cassert>
#include <cmath>

#include "market/models/random_walk_model.hpp"

int main() {
    market::RandomWalkModel model (
        0.0,
        1.0,
        12345
    );
    double price = 100.00;

    for (int i = 0; i < 1000; i++) {
        price = model.next_price(price);
        assert(std::isfinite(price));
    }
    return 0;
}
