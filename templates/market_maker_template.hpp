#pragma once

// General market-maker template for the market-model framework.
//
// Copy to include/market/agents/ and adapt. Each step it:
//   1. cancels its old quotes
//   2. estimates a reference price and recent volatility
//   3. decides whether it is safe to quote at all (kill switch)
//   4. computes a quote centre (reference shifted against inventory)
//   5. computes a half-spread (base + volatility term)
//   6. posts one or more bid/ask levels, respecting inventory limits
//   7. settles any trades that happen immediately
//
// Every decision lives in a small protected method so a subclass can replace
// just that piece (e.g. a different reference price or spread rule).

#include <cmath>
#include <cstddef>
#include <deque>

#include "market/agents/agent.hpp"
#include "market/agents/agent_utils.hpp"
#include "market/agents/trader_registry.hpp"
#include "market/core/units.hpp"
#include "market/market/market.hpp"
#include "market/market/settlement.hpp"
#include "market/models/fundamental_value.hpp"

namespace market {

struct MarketMakerParams {
    // --- spread ---------------------------------------------------------
    Price base_half_spread = 5;          // ticks; must cover fees
    double volatility_multiplier = 0.0;  // extra half-spread = mult * vol * price
    std::size_t volatility_window = 20;  // steps used for rolling volatility

    // --- inventory ------------------------------------------------------
    double inventory_skew = 0.01;        // ticks the quotes shift per unit held
    Position max_inventory = 50;         // |position| cap per side

    // --- size and depth -------------------------------------------------
    Quantity quote_size = 5;             // per level
    std::size_t levels = 1;              // number of price levels per side
    Price level_spacing = 2;             // ticks between levels

    // --- safety ---------------------------------------------------------
    double max_deviation = 0.25;         // stop quoting if price is this far
                                         // (as a fraction) from fundamental
};

class TemplateMarketMaker : public Agent {
public:
    TemplateMarketMaker(TraderId trader_id, MarketMakerParams params = {})
        : trader_id_(trader_id), p_(params) {}

    void step(Timestamp timestamp,
              Market& market,
              TraderRegistry& traders,
              Settlement& settlement,
              FundamentalValue& fundamental_value) override {
        Trader* trader = traders.find_trader(trader_id_);
        if (trader == nullptr) { return; }

        // 1. remove stale quotes
        market.cancel_all_orders(trader_id_);

        // 2. observe
        Price ref = reference(market, fundamental_value);
        update_history(ref);
        double vol = rolling_volatility();

        // 3. kill switch
        if (!safe_to_quote(ref, fundamental_value.value())) { return; }

        // 4-5. price the quotes
        double centre = quote_centre(ref, *trader);
        double half = half_spread(ref, vol);

        // 6. post levels
        for (std::size_t level = 0; level < p_.levels; ++level) {
            double offset = half + static_cast<double>(level * p_.level_spacing);
            Quantity size = quote_size(level);

            if (may_buy(*trader, size)) {
                Price bid = to_price(centre - offset);
                submit(Side::Buy, bid, size, timestamp, market, settlement);
            }
            if (may_sell(*trader, size)) {
                Price ask = to_price(centre + offset);
                if (ask <= to_price(centre - offset)) { ask += 1; }  // never cross own bid
                submit(Side::Sell, ask, size, timestamp, market, settlement);
            }
        }
    }

protected:
    // ---- replaceable decisions ------------------------------------------

    // What price are we quoting around?
    virtual Price reference(const Market& market, FundamentalValue& fundamental) {
        return reference_price(market, fundamental.value());
        // alternative: blend book and fundamental
        //   w * mid + (1 - w) * fundamental
    }

    // Shift quotes against inventory: long -> lower, short -> higher.
    virtual double quote_centre(Price ref, const Trader& trader) {
        return static_cast<double>(ref)
             - p_.inventory_skew * static_cast<double>(trader.asset_quantity());
    }

    // Wider when the market is volatile.
    virtual double half_spread(Price ref, double volatility) {
        return static_cast<double>(p_.base_half_spread)
             + p_.volatility_multiplier * volatility * static_cast<double>(ref);
    }

    // Size per level (e.g. larger further from the touch).
    virtual Quantity quote_size(std::size_t /*level*/) { return p_.quote_size; }

    // Inventory limits: stop adding to a position that is already too big.
    virtual bool may_buy(const Trader& t, Quantity size) {
        return t.asset_quantity() + static_cast<Position>(size) <= p_.max_inventory;
    }
    virtual bool may_sell(const Trader& t, Quantity size) {
        return t.asset_quantity() - static_cast<Position>(size) >= -p_.max_inventory;
    }

    // Stop quoting when the market looks broken (news shock, crash...).
    virtual bool safe_to_quote(Price ref, Price fundamental) {
        double dev = std::fabs(static_cast<double>(ref) - static_cast<double>(fundamental))
                   / static_cast<double>(fundamental);
        return dev <= p_.max_deviation;
    }

    TraderId trader_id_;
    MarketMakerParams p_;

private:
    void submit(Side side, Price price, Quantity size, Timestamp t,
                Market& market, Settlement& settlement) {
        Order order;
        order.trader_id = trader_id_;
        order.side = side;
        order.type = OrderType::Limit;
        order.price = price;
        order.quantity = size;
        order.timestamp = t;

        // The market rejects the quote if the trader cannot fund it.
        for (const Trade& trade : market.submit_order(order)) {
            settlement.settle(trade);
        }
    }

    void update_history(Price ref) {
        history_.push_back(static_cast<double>(ref));
        if (history_.size() > p_.volatility_window + 1) { history_.pop_front(); }
    }

    // Std. dev. of per-step log returns over the window.
    double rolling_volatility() const {
        if (history_.size() < 3) { return 0.0; }
        double sum = 0.0, sum_sq = 0.0;
        std::size_t n = history_.size() - 1;
        for (std::size_t i = 1; i < history_.size(); ++i) {
            double r = std::log(history_[i] / history_[i - 1]);
            sum += r;
            sum_sq += r * r;
        }
        double mean = sum / static_cast<double>(n);
        double var = sum_sq / static_cast<double>(n) - mean * mean;
        return var > 0.0 ? std::sqrt(var) : 0.0;
    }

    std::deque<double> history_;
};

}  // namespace market
