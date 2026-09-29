#include "market/market/settlement.hpp"

namespace market {

Settlement::Settlement(TraderRegistry& traders) : traders_(traders) {}

bool Settlement::settle(const Trade& trade) {
    Trader* buyer = traders_.find_trader(trade.buyer_id);
    Trader* seller = traders_.find_trader(trade.seller_id);

    if (buyer == nullptr || seller == nullptr) { return false; }

    double value = trade.value();

    // Check both sides first so a failure cannot leave a half-settled trade.
    if (buyer->reserved_cash() + 1e-9 < value || seller->reserved_assets() < trade.quantity) {
        return false;
    }

    if (!buyer->consume_reserved_cash(value)) { return false; }
    if (!seller->consume_reserved_assets(trade.quantity)) { return false; }

    buyer->add_assets(trade.quantity);
    seller->add_cash(value);

    return true;
}

}  // namespace market
