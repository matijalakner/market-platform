#include <cassert>

#include "market/agents/trader_registry.hpp"
#include "market/market/settlement.hpp"

int main() {
    market::Trader buyer(1, 10000.0, 100);
    market::Trader seller(2, 5000.0, 200);

    // Settlement spends reserved funds, so reserve them first.
    assert(buyer.reserve_cash(2000.0));
    assert(seller.reserve_assets(20));

    market::TraderRegistry registry;
    registry.add_trader(buyer);
    registry.add_trader(seller);

    market::Settlement settlement(registry);

    market::Trade trade{
        .buy_order_id = 10,
        .sell_order_id = 20,
        .buyer_id = 1,
        .seller_id = 2,
        .price = 100.0,
        .quantity = 20,
        .timestamp = 1
    };

    bool success = settlement.settle(trade);

    assert(success);

    auto* buyer_ptr = registry.find_trader(1);
    auto* seller_ptr = registry.find_trader(2);

    assert(buyer_ptr != nullptr);
    assert(seller_ptr != nullptr);
    assert(buyer_ptr->cash() == 8000.0);
    assert(buyer_ptr->asset_quantity() == 120);
    assert(seller_ptr->cash() == 7000.0);
    assert(seller_ptr->asset_quantity() == 180);

    // Nothing reserved any more, so a second settlement must fail.
    assert(!settlement.settle(trade));

    return 0;
}
