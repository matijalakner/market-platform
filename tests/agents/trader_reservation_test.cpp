#include <cassert>

#include "market/agents/trader.hpp"

int main() {
    // Cash test:
    {
        market::Trader trader(1, 1000.0, 100);

        assert(trader.cash() == 1000.0);
        assert(trader.available_cash() == 1000.0);

        bool reserved = trader.reserve_cash(600.0);

        assert(reserved);

        assert(trader.cash() == 1000.0);
        assert(trader.reserved_cash() == 600.0);
        assert(trader.available_cash() == 400.0);

        bool second_reservation = trader.reserve_cash(500.0);

        assert(!second_reservation);
        assert(trader.available_cash() == 400.0);

        bool consumed = trader.consume_reserved_cash(200.0);

        assert(consumed);
        assert(trader.cash() == 800.0);
        assert(trader.reserved_cash() == 400.0);
        assert(trader.available_cash() == 400.0);

        bool released = trader.release_cash(400.0);

        assert(released);
        assert(trader.cash() == 800.0);
        assert(trader.reserved_cash() == 0.0);
        assert(trader.available_cash() == 800.0);

        assert(!trader.release_cash(1.0));  // nothing reserved
    }

    // Asset test:
    {
        market::Trader trader(2, 0.0, 100);

        assert(trader.available_assets() == 100);

        bool reserved = trader.reserve_assets(60);

        assert(reserved);

        assert(trader.asset_quantity() == 100);
        assert(trader.reserved_assets() == 60);
        assert(trader.available_assets() == 40);

        bool consumed = trader.consume_reserved_assets(20);

        assert(consumed);

        assert(trader.asset_quantity() == 80);
        assert(trader.reserved_assets() == 40);
        assert(trader.available_assets() == 40);

        assert(trader.release_assets(40));
        assert(trader.available_assets() == 80);
    }

    return 0;
}
