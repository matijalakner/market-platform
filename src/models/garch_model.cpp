#include "market/models/garch_model.hpp"

#include <cmath>

#include "market/core/units.hpp"

namespace market {

GarchModel::GarchModel(double mu, double omega, double alpha, double beta, std::uint64_t seed)
    : mu_(mu),
      omega_(omega),
      alpha_(alpha),
      beta_(beta),
      // start at the long-run variance when it exists
      variance_((alpha + beta < 1.0) ? omega / (1.0 - alpha - beta) : omega),
      generator_(seed),
      normal_(0.0, 1.0) {}

double GarchModel::current_volatility() const { return std::sqrt(variance_); }

Price GarchModel::next_price(Price current_price) {
    double shock = std::sqrt(variance_) * normal_(generator_);
    double log_return = mu_ + shock;
    variance_ = omega_ + alpha_ * shock * shock + beta_ * variance_;
    return to_price(static_cast<double>(current_price) * std::exp(log_return));
}

}  // namespace market
