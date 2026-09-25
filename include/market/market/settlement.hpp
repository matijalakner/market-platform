#pragma once

#include "market/agents/trader_registry.hpp"
#include "market/market/trade.hpp"

namespace market {

class Settlement {
public:
    explicit Settlement(TraderRegistry& traders);
    bool settle(const Trade& trade);

private:
    TraderRegistry& traders_;
};
}