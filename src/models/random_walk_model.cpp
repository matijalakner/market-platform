#include "market/models/random_walk_model.hpp"

#include "market/core/units.hpp"

namespace market {

RandomWalkModel::RandomWalkModel(double drift, double volatility, std::uint64_t seed)
    : drift_(drift),
      volatility_(volatility),
      generator_(seed),
      normal_(0.0, 1.0) {}

Price RandomWalkModel::next_price(Price current_price) {
    double change = drift_ + volatility_ * normal_(generator_);
    return to_price(static_cast<double>(current_price) + change);
}

}  // namespace market
