#pragma once

#include "trader.hpp"
#include "market/agents/agent.hpp"
#include "market/agents/trader.hpp"

namespace market {
    class RandomTrader : public Agent {
    public:
        explicit RandomTrader(Trader trader);
        void step(Timestamp timestamp, Market& market) override;
        Trader& trader();
    private:
        Trader trader_;
    };
}