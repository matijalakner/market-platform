#include "market/models/random_walk_model.hpp"

namespace market {

RandomWalkModel::RandomWalkModel(
    double drift,
    double volatility,
    std::uint64_t seed
)
    : drift_(drift),
      volatility_(volatility),
      generator_(seed),
      normal_(0.0, 1.0) {}

Price RandomWalkModel::next_price(Price current_price) {
    double random_shock = normal_(generator_);
    double change = drift_ + volatility_ * random_shock;
    return current_price + change;
}

}  // namespace market
