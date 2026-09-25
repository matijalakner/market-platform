#include "market/agents/random_trader.hpp"

namespace market {
    RandomTrader::RandomTrader(Trader trader) : trader_(trader) {}
    void RandomTrader::step(Timestamp timestamp, Market& market) {
        #NOTE Implement trading logic
        (void)timestamp;
        (void)market;
    }
    Trader& RandomTrader::trader() { return trader_; }
}