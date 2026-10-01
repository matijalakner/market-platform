#pragma once

#include <cstdint>
#include <random>

#include "market/models/price_model.hpp"

namespace market {

// Additive random walk in ticks: price + drift + volatility * N(0, 1).
// The result is rounded to a whole tick and never drops below one tick.
class RandomWalkModel : public PriceModel {
public:
    RandomWalkModel(double drift, double volatility, std::uint64_t seed);

    Price next_price(Price current_price) override;

private:
    double drift_;
    double volatility_;
    std::mt19937_64 generator_;
    std::normal_distribution<double> normal_;
};

}  // namespace market
