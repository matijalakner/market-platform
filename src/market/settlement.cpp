#include "../../include/market/market/settlement.hpp"
#include "market/market/settlement.hpp"

namespace market {
    Settlement::Settlement(TraderRegistry& traders) : traders_(traders) {}
    bool Settlement::settle(const Trade& trade) {
        Trader* buyer = traders_.find_trader(trade.buyer_id);
        Trader* seller = traders_.find_trader(trade.seller_id);

        if (buyer == nullptr || seller == nullptr) { return false; }

        double value = trade.price * static_cast<double>(trade.quantity);

        if (buyer->cash < value) { return false; }
        if (seller->asset_quantity() < value) { return false; }

        buyer->remove_cash(value);
        buyer->add_assets(trade.quantity);

        seller->add_cash(value);
        seller->remove_assets(trade.quantity);

        return true;
    }
}
