#pragma once

#include <cstddef>
#include <unordered_map>

#include "trader.hpp"
#include "market/agents/trader.hpp"

namespace market {
    class TraderRegistry {
    public:
        void add_order(Trader trader);
        Trader* find_trader(TraderId id);
        const Trader* find_trader(TraderId id) const;
        std::size_t size() const;
    private:
        std::unordered_map<TraderId, Trader> traders_;
    };
}