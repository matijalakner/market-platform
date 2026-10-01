#include <cassert>

#include "market/agents/trader.hpp"

int main() {
    // Without a short limit a trader can only sell what it owns.
    market::Trader plain(1, 0.0, 10);
    assert(plain.available_assets() == 10);
    assert(!plain.reserve_assets(11));

    // With a short limit it can sell beyond its holdings, up to the limit.
    market::Trader shorter(2, 0.0, 10, 50);
    assert(shorter.short_limit() == 50);
    assert(shorter.available_assets() == 60);
    assert(!shorter.reserve_assets(61));
    assert(shorter.reserve_assets(40));
    assert(shorter.consume_reserved_assets(40));

    // Sold 40 with only 10 owned: now short 30, so 20 more may be sold.
    assert(shorter.asset_quantity() == -30);
    assert(shorter.available_assets() == 20);
    assert(!shorter.reserve_assets(21));

    // Buying back reduces the short.
    shorter.add_assets(30);
    assert(shorter.asset_quantity() == 0);
    assert(shorter.available_assets() == 50);

    return 0;
}
