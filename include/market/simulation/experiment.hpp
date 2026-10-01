#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "market/core/config.hpp"
#include "market/core/types.hpp"
#include "market/statistics/statistics.hpp"

namespace market {

// Everything needed to run one experiment. Money values (prices, cash,
// spreads, volatilities of the random walk) are in currency; they are
// converted to integer ticks using `simulation.tick_size` when the run starts.
struct ExperimentConfig {
    SimulationConfig simulation;
    MarketConfig market;

    double initial_price = 100.0;

    // Starting endowment of every trader.
    double initial_cash = 10000.0;
    Position initial_assets = 100;
    Quantity short_limit = 0;

    struct Model {
        std::string type = "random_walk";   // random_walk | gbm | garch
        double drift = 0.0;                 // random_walk: currency per step
        double volatility = 0.05;           // random_walk: currency per step
        double mu = 0.0;                    // gbm / garch: fraction per step
        double sigma = 0.001;               // gbm: fraction per step
        double omega = 1e-7;                // garch
        double alpha = 0.1;                 // garch
        double beta = 0.85;                 // garch
    } model;

    struct Agents {
        std::size_t random_traders = 10;
        std::size_t fundamental_traders = 5;
        std::size_t market_makers = 1;
        std::size_t momentum_traders = 0;
        std::size_t noise_traders = 0;
        std::size_t arbitrage_traders = 0;
    } agents;

    struct Params {
        double fundamental_threshold = 0.02;
        Quantity order_quantity = 5;
        double market_maker_half_spread = 0.05;   // currency
        Quantity market_maker_quantity = 5;
        double market_maker_skew = 0.001;         // currency per unit of inventory
        std::size_t momentum_lookback = 10;
        double momentum_threshold = 0.005;
        double noise_max_offset = 0.10;           // currency
        double noise_market_order_probability = 0.1;
        double arbitrage_threshold = 0.01;
    } params;

    struct News {
        Timestamp time = 0;
        double shock = 0.0;   // relative, -0.05 = -5%
    };
    std::vector<News> news;

    // Empty directory = don't write CSV files.
    std::string output_directory;
    std::size_t trader_interval = 1;   // per-trader rows every N steps (0 = none)
};

struct ExperimentResult {
    StatisticsSummary summary;
    std::size_t trades = 0;
    double total_fees = 0.0;
};

// Throws std::runtime_error on malformed JSON or an unreadable file.
ExperimentConfig parse_experiment(const std::string& json_text);
ExperimentConfig load_experiment(const std::string& path);

// Builds traders, agents and model from the config, runs the simulation and,
// if output_directory is set, writes market.csv and traders.csv there.
ExperimentResult run_experiment(const ExperimentConfig& config);

}  // namespace market
