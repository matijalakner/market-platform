#pragma once

#include "market/core/types.hpp"

namespace market {

class FundamentalValue
{
public:
    explicit FundamentalValue(Price initial_value);
    Price value() const;
    void set_value(Price value);

private:
    Price value_;
};
}