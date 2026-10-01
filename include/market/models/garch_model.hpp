#pragma once

#include <cstdint>
#include <random>

#include "market/models/price_model.hpp"

namespace market {

// Volatility model (GARCH(1,1)) with multiplicative price moves:
//   r_t      = mu + sigma_t * z_t,                z_t ~ N(0, 1)
//   sigma_t+1^2 = omega + alpha * (r_t - mu)^2 + beta * sigma_t^2
// Volatility clusters: big moves are followed by more big moves.
// Requires alpha + beta < 1 for a finite long-run variance.
class GarchModel : public PriceModel {
public:
    GarchModel(double mu, double omega, double alpha, double beta, std::uint64_t seed);

    Price next_price(Price current_price) override;

    double current_volatility() const;

private:
    double mu_;
    double omega_;
    double alpha_;
    double beta_;
    double variance_;
    std::mt19937_64 generator_;
    std::normal_distribution<double> normal_;
};

}  // namespace market
