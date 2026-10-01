#include "market/statistics/statistics.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <stdexcept>

#include "market/core/units.hpp"

namespace market {

StatisticsRecorder::StatisticsRecorder(double tick_size, std::size_t trader_interval)
    : tick_size_(tick_size), trader_interval_(trader_interval) {}

void StatisticsRecorder::record(
    Timestamp timestamp,
    const Market& market,
    const TraderRegistry& traders,
    Price fundamental_value
) {
    Price mark = fundamental_value;
    if (market.has_traded()) {
        mark = market.last_trade_price();
    } else if (auto mid = market.mid_price()) {
        mark = to_price(*mid);
    }

    auto to_currency = [this](double ticks) { return ticks * tick_size_; };

    MarketSnapshot snap;
    snap.timestamp = timestamp;
    snap.price = to_currency(static_cast<double>(mark));
    snap.fundamental = to_currency(static_cast<double>(fundamental_value));
    if (auto bid = market.best_bid()) { snap.bid = to_currency(static_cast<double>(*bid)); }
    if (auto ask = market.best_ask()) { snap.ask = to_currency(static_cast<double>(*ask)); }
    if (auto mid = market.mid_price()) { snap.mid = to_currency(*mid); }
    if (auto spread = market.spread()) { snap.spread = to_currency(static_cast<double>(*spread)); }
    snap.total_volume = market.total_volume();
    snap.step_volume = snap.total_volume - last_total_volume_;
    last_total_volume_ = snap.total_volume;
    snap.bid_depth = market.order_book().bid_depth();
    snap.ask_depth = market.order_book().ask_depth();
    market_.push_back(snap);

    if (trader_interval_ != 0 && records_ % trader_interval_ == 0) {
        std::vector<TraderId> ids;
        ids.reserve(traders.size());
        for (const auto& entry : traders.traders()) { ids.push_back(entry.first); }
        std::sort(ids.begin(), ids.end());

        for (TraderId id : ids) {
            const Trader* trader = traders.find_trader(id);
            TraderSnapshot row;
            row.timestamp = timestamp;
            row.trader_id = id;
            row.cash = to_currency(trader->cash());
            row.inventory = trader->asset_quantity();
            row.pnl = to_currency(trader->pnl(mark));
            traders_.push_back(row);
        }
    }
    ++records_;
}

const std::vector<MarketSnapshot>& StatisticsRecorder::market_snapshots() const { return market_; }
const std::vector<TraderSnapshot>& StatisticsRecorder::trader_snapshots() const { return traders_; }

std::vector<double> StatisticsRecorder::log_returns() const {
    std::vector<double> returns;
    for (std::size_t i = 1; i < market_.size(); ++i) {
        double previous = market_[i - 1].price;
        double current = market_[i].price;
        if (previous > 0.0 && current > 0.0) { returns.push_back(std::log(current / previous)); }
    }
    return returns;
}

double StatisticsRecorder::volatility() const {
    std::vector<double> returns = log_returns();
    if (returns.size() < 2) { return 0.0; }

    double mean = 0.0;
    for (double r : returns) { mean += r; }
    mean /= static_cast<double>(returns.size());

    double sum_sq = 0.0;
    for (double r : returns) { sum_sq += (r - mean) * (r - mean); }
    return std::sqrt(sum_sq / static_cast<double>(returns.size() - 1));
}

StatisticsSummary StatisticsRecorder::summary() const {
    StatisticsSummary s;
    s.steps = market_.size();
    if (market_.empty()) { return s; }

    s.first_price = market_.front().price;
    s.last_price = market_.back().price;
    s.total_return = s.first_price > 0.0 ? s.last_price / s.first_price - 1.0 : 0.0;
    s.volatility = volatility();
    s.total_volume = market_.back().total_volume;

    double spread_sum = 0.0, bid_sum = 0.0, ask_sum = 0.0, mis_sum = 0.0;
    std::size_t spread_count = 0;
    for (const MarketSnapshot& m : market_) {
        if (m.spread) { spread_sum += *m.spread; ++spread_count; }
        bid_sum += static_cast<double>(m.bid_depth);
        ask_sum += static_cast<double>(m.ask_depth);
        if (m.fundamental > 0.0) { mis_sum += std::fabs(m.price - m.fundamental) / m.fundamental; }
    }
    double n = static_cast<double>(market_.size());
    s.mean_spread = spread_count ? spread_sum / static_cast<double>(spread_count) : 0.0;
    s.mean_bid_depth = bid_sum / n;
    s.mean_ask_depth = ask_sum / n;
    s.mean_abs_mispricing = mis_sum / n;
    return s;
}

namespace {

void write_optional(std::ostream& out, const std::optional<double>& value) {
    if (value) { out << *value; }
}

}  // namespace

void StatisticsRecorder::write_market_csv(const std::string& path) const {
    std::ofstream out(path);
    if (!out) { throw std::runtime_error("cannot open " + path); }
    out << std::fixed << std::setprecision(6);
    out << "timestamp,price,fundamental,bid,ask,mid,spread,volume,total_volume,bid_depth,ask_depth\n";
    for (const MarketSnapshot& m : market_) {
        out << m.timestamp << ',' << m.price << ',' << m.fundamental << ',';
        write_optional(out, m.bid);    out << ',';
        write_optional(out, m.ask);    out << ',';
        write_optional(out, m.mid);    out << ',';
        write_optional(out, m.spread); out << ',';
        out << m.step_volume << ',' << m.total_volume << ','
            << m.bid_depth << ',' << m.ask_depth << '\n';
    }
}

void StatisticsRecorder::write_traders_csv(const std::string& path) const {
    std::ofstream out(path);
    if (!out) { throw std::runtime_error("cannot open " + path); }
    out << std::fixed << std::setprecision(6);
    out << "timestamp,trader_id,cash,inventory,pnl\n";
    for (const TraderSnapshot& t : traders_) {
        out << t.timestamp << ',' << t.trader_id << ',' << t.cash << ','
            << t.inventory << ',' << t.pnl << '\n';
    }
}

}  // namespace market
