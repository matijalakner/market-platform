#pragma once

#include"market/code/types.hpp"

namespace market {
    class Market;
    class Agent {
        public:
            virtual ~Agent() = default;
            virtual void step(
                Timestamp timestamp,
                Market& market
            ) = 0;
    };
}