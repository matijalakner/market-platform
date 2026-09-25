#pragma once

#include <random>
#include "market/models/price_models.hpp"

namespace market {
    class RandomWalkModel : public PriceModel {
    public:
        RandomWalkModel(
            double drift,
            double volatility,
            std::unint64_t seed
        );
        Price next_price(Price current_price) override;

    private:
        double drift_;
        double volatility_;
        std::mt19937_64 generator_;
        str::normal_distribution<double> normal_;
    };
}