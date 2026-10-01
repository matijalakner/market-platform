#include "market/models/geometric_brownian_model.hpp"

#include <cmath>

#include "market/core/units.hpp"

namespace market {

GeometricBrownianModel::GeometricBrownianModel(double mu, double sigma, std::uint64_t seed)
    : mu_(mu), sigma_(sigma), generator_(seed), normal_(0.0, 1.0) {}

Price GeometricBrownianModel::next_price(Price current_price) {
    double log_return = (mu_ - 0.5 * sigma_ * sigma_) + sigma_ * normal_(generator_);
    return to_price(static_cast<double>(current_price) * std::exp(log_return));
}

}  // namespace market
