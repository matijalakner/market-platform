#include <cassert>

#include "market/agents/trader.hpp"

int main() {
    market::Trader trader(1, 1000.0, 10);

    // Unchanged holdings: no P&L at any mark price.
    assert(trader.pnl(100) == 0.0);
    assert(trader.pnl(500) == 0.0);

    // Buy 5 at 100: cash -500, assets +5. Marking at 120 gives +100.
    assert(trader.reserve_cash(500.0));
    assert(trader.consume_reserved_cash(500.0));
    trader.add_assets(5);

    assert(trader.equity(120) == 500.0 + 15 * 120.0);
    assert(trader.pnl(100) == 0.0);
    assert(trader.pnl(120) == 100.0);
    assert(trader.pnl(80) == -100.0);

    // Fees reduce P&L and never push cash below zero.
    assert(trader.charge_fee(10.0) == 10.0);
    assert(trader.pnl(100) == -10.0);
    assert(trader.charge_fee(1e9) == 490.0);
    assert(trader.cash() == 0.0);

    return 0;
}
