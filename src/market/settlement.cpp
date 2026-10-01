#include "market/market/settlement.hpp"

namespace market {

Settlement::Settlement(TraderRegistry& traders, double fee_rate)
    : traders_(traders), fee_rate_(fee_rate) {}

double Settlement::fee_rate() const { return fee_rate_; }
double Settlement::total_fees() const { return total_fees_; }

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

    if (fee_rate_ > 0.0) {
        double fee = value * fee_rate_;
        total_fees_ += buyer->charge_fee(fee);
        total_fees_ += seller->charge_fee(fee);
    }

    return true;
}

}  // namespace market
