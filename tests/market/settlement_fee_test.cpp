#include <cassert>
#include <cmath>

#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"
#include "market/market/settlement.hpp"

int main() {
    market::TraderRegistry traders;
    traders.add_trader(market::Trader(1, 10000.0, 0));
    traders.add_trader(market::Trader(2, 0.0, 100));

    market::Market market(traders);
    market::Settlement settlement(traders, 0.001);   // 0.1% each side

    market::Order sell;
    sell.trader_id = 2; sell.side = market::Side::Sell; sell.type = market::OrderType::Limit;
    sell.price = 100; sell.quantity = 20; sell.timestamp = 1;
    market.submit_order(sell);

    market::Order buy = sell;
    buy.trader_id = 1; buy.side = market::Side::Buy; buy.timestamp = 2;
    auto trades = market.submit_order(buy);
    assert(trades.size() == 1);
    assert(settlement.settle(trades[0]));

    // value 2000, fee 2.0 per side
    assert(std::fabs(traders.find_trader(1)->cash() - (10000.0 - 2000.0 - 2.0)) < 1e-9);
    assert(std::fabs(traders.find_trader(2)->cash() - (2000.0 - 2.0)) < 1e-9);
    assert(std::fabs(settlement.total_fees() - 4.0) < 1e-9);
    assert(settlement.fee_rate() == 0.001);

    return 0;
}
