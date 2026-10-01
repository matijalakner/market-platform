#pragma once

#include <cstdint>
#include <random>

#include "market/models/price_model.hpp"

namespace market {

// Multiplicative model: price * exp((mu - sigma^2 / 2) + sigma * N(0, 1)).
// mu and sigma are per-step fractions (e.g. sigma = 0.001 is 0.1% per step),
// so moves scale with the price level and the price stays positive.
class GeometricBrownianModel : public PriceModel {
public:
    GeometricBrownianModel(double mu, double sigma, std::uint64_t seed);

    Price next_price(Price current_price) override;

private:
    double mu_;
    double sigma_;
    std::mt19937_64 generator_;
    std::normal_distribution<double> normal_;
};

}  // namespace market
