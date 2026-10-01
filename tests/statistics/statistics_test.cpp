#include <cassert>
#include <cmath>
#include <fstream>
#include <sstream>

#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"
#include "market/market/settlement.hpp"
#include "market/statistics/statistics.hpp"

namespace {
market::Order limit(market::TraderId trader, market::Side side, market::Price price,
                    market::Quantity qty) {
    market::Order o;
    o.trader_id = trader; o.side = side; o.type = market::OrderType::Limit;
    o.price = price; o.quantity = qty; o.timestamp = 1;
    return o;
}
}  // namespace

int main() {
    market::TraderRegistry traders;
    traders.add_trader(market::Trader(1, 1e6, 100));
    traders.add_trader(market::Trader(2, 1e6, 100));
    market::Market market(traders);
    market::Settlement settlement(traders);

    market::StatisticsRecorder recorder(0.01, 1);

    // t=1: empty book -> price falls back to the fundamental value.
    recorder.record(1, market, traders, 10000);

    // t=2: a two-sided book.
    market.submit_order(limit(1, market::Side::Buy, 9990, 10));
    market.submit_order(limit(2, market::Side::Sell, 10010, 20));
    recorder.record(2, market, traders, 10000);

    // t=3: a trade happens.
    for (auto& t : market.submit_order(limit(1, market::Side::Buy, 10010, 5))) { settlement.settle(t); }
    recorder.record(3, market, traders, 10000);

    const auto& snaps = recorder.market_snapshots();
    assert(snaps.size() == 3);

    assert(std::fabs(snaps[0].price - 100.0) < 1e-9);          // 10000 ticks * 0.01
    assert(!snaps[0].bid.has_value());

    assert(std::fabs(*snaps[1].bid - 99.90) < 1e-9);
    assert(std::fabs(*snaps[1].ask - 100.10) < 1e-9);
    assert(std::fabs(*snaps[1].mid - 100.00) < 1e-9);
    assert(std::fabs(*snaps[1].spread - 0.20) < 1e-9);
    assert(snaps[1].bid_depth == 10);
    assert(snaps[1].ask_depth == 20);
    assert(snaps[1].step_volume == 0);

    assert(std::fabs(snaps[2].price - 100.10) < 1e-9);         // last trade
    assert(snaps[2].step_volume == 5);
    assert(snaps[2].total_volume == 5);
    assert(snaps[2].ask_depth == 15);

    // Per-trader rows: 2 traders * 3 records, sorted by id.
    const auto& rows = recorder.trader_snapshots();
    assert(rows.size() == 6);
    assert(rows[0].trader_id == 1 && rows[1].trader_id == 2);
    // Trader 1 bought 5 at 10010 and the mark is 10010: no loss at the trade price.
    assert(rows[4].inventory == 105);
    assert(std::fabs(rows[4].pnl) < 1e-6);

    // Returns and summary.
    auto returns = recorder.log_returns();
    assert(returns.size() == 2);
    assert(std::fabs(returns[0]) < 1e-12);
    assert(returns[1] > 0.0);
    assert(recorder.volatility() > 0.0);

    auto s = recorder.summary();
    assert(s.steps == 3);
    assert(s.total_volume == 5);
    assert(s.total_return > 0.0);
    assert(s.mean_spread > 0.0);

    // CSV output.
    recorder.write_market_csv("/tmp/mm_stats_market.csv");
    recorder.write_traders_csv("/tmp/mm_stats_traders.csv");
    std::ifstream in("/tmp/mm_stats_market.csv");
    std::string header, line1;
    std::getline(in, header);
    std::getline(in, line1);
    assert(header.rfind("timestamp,price,fundamental,bid,ask", 0) == 0);
    assert(line1.find(",,,") != std::string::npos);            // missing bid/ask/mid left blank

    // Per-trader recording can be switched off.
    market::StatisticsRecorder none(0.01, 0);
    none.record(1, market, traders, 10000);
    assert(none.trader_snapshots().empty());

    return 0;
}
