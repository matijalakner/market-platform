#include <cassert>

#include "market/agents/trader.hpp"

int main() {
    market::Trader trader(
        1,
        10000.0,
        100
    );

    assert(trader.id() == 1);
    assert(trader.cash() == 10000.0);
    assert(trader.asset_quantity() == 100);

    trader.remove_cash(2000.0);
    trader.add_assets(20);

    assert(trader.cash() == 8000.0);
    assert(trader.asset_quantity() == 120);

    trader.add_cash(1000.0);
    trader.remove_asset(10);

    assert(trader.cash() == 9000.0);
    assert(trader.asset_quantity() == 110);

    return 0;
}