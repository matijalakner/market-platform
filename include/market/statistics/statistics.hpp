#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "market/agents/trader_registry.hpp"
#include "market/core/types.hpp"
#include "market/market/market.hpp"

namespace market {

// All prices and money amounts are in currency (ticks * tick_size).
struct MarketSnapshot {
    Timestamp timestamp = 0;
    double price = 0.0;            // last trade, else mid, else fundamental
    double fundamental = 0.0;
    std::optional<double> bid;
    std::optional<double> ask;
    std::optional<double> mid;
    std::optional<double> spread;
    Quantity step_volume = 0;
    Quantity total_volume = 0;
    Quantity bid_depth = 0;        // total resting quantity
    Quantity ask_depth = 0;
};

struct TraderSnapshot {
    Timestamp timestamp = 0;
    TraderId trader_id = 0;
    double cash = 0.0;
    Position inventory = 0;
    double pnl = 0.0;              // mark-to-market profit/loss
};

struct StatisticsSummary {
    std::size_t steps = 0;
    double first_price = 0.0;
    double last_price = 0.0;
    double total_return = 0.0;         // last / first - 1
    double volatility = 0.0;           // std. dev. of per-step log returns
    double mean_spread = 0.0;
    double mean_bid_depth = 0.0;
    double mean_ask_depth = 0.0;
    double mean_abs_mispricing = 0.0;  // mean |price - fundamental| / fundamental
    Quantity total_volume = 0;
};

class StatisticsRecorder {
public:
    // `trader_interval`: record per-trader rows every N steps (0 = never).
    explicit StatisticsRecorder(double tick_size = 1.0, std::size_t trader_interval = 1);

    void record(
        Timestamp timestamp,
        const Market& market,
        const TraderRegistry& traders,
        Price fundamental_value
    );

    const std::vector<MarketSnapshot>& market_snapshots() const;
    const std::vector<TraderSnapshot>& trader_snapshots() const;

    // Per-step log returns of the recorded price series.
    std::vector<double> log_returns() const;
    double volatility() const;
    StatisticsSummary summary() const;

    void write_market_csv(const std::string& path) const;
    void write_traders_csv(const std::string& path) const;

private:
    double tick_size_;
    std::size_t trader_interval_;
    std::size_t records_ = 0;
    Quantity last_total_volume_ = 0;
    std::vector<MarketSnapshot> market_;
    std::vector<TraderSnapshot> traders_;
};

}  // namespace market
