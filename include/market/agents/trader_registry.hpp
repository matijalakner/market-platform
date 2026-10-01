#pragma once

#include <cstddef>
#include <unordered_map>

#include "market/agents/trader.hpp"

namespace market {

class TraderRegistry {
public:
    // Returns false if a trader with the same id already exists.
    bool add_trader(const Trader& trader);
    Trader* find_trader(TraderId id);
    const Trader* find_trader(TraderId id) const;
    std::size_t size() const;
    const std::unordered_map<TraderId, Trader>& traders() const;

private:
    std::unordered_map<TraderId, Trader> traders_;
};

}  // namespace market
