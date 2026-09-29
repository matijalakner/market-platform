#include "market/agents/trader_registry.hpp"

namespace market {

bool TraderRegistry::add_trader(const Trader& trader) {
    auto result = traders_.emplace(trader.id(), trader);
    return result.second;
}

Trader* TraderRegistry::find_trader(TraderId id) {
    auto it = traders_.find(id);
    if (it == traders_.end()) { return nullptr; }
    return &it->second;
}

const Trader* TraderRegistry::find_trader(TraderId id) const {
    auto it = traders_.find(id);
    if (it == traders_.end()) { return nullptr; }
    return &it->second;
}

std::size_t TraderRegistry::size() const {
    return traders_.size();
}

}  // namespace market
