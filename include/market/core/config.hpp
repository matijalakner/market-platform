#pragma once

#include <cstddef>

namespace market {
    struct SimuationConfig {
        std::size_t steps = 1000;
        std::size_t trader_count = 100;
    };
}