#pragma once

#include "market/core/types.hpp"

namespace market {

class Trader {
public:
    // `short_limit` is the number of assets the trader may sell that it does
    // not own. 0 (the default) disables short selling.
    Trader(TraderId id, double cash, Position asset_quantity, Quantity short_limit = 0);

    TraderId id() const;

    double cash() const;
    double available_cash() const;
    double reserved_cash() const;

    // Signed: negative when the trader is short.
    Position asset_quantity() const;
    Quantity available_assets() const;
    Quantity reserved_assets() const;
    Quantity short_limit() const;

    void add_cash(double amount);
    bool remove_cash(double amount);

    bool reserve_cash(double amount);
    bool release_cash(double amount);

    void add_assets(Quantity quantity);
    bool remove_assets(Quantity quantity);

    bool reserve_assets(Quantity quantity);
    bool release_assets(Quantity quantity);

    // Spend reserved cash / hand over reserved assets (used when settling).
    bool consume_reserved_cash(double amount);
    bool consume_reserved_assets(Quantity quantity);

    // Deducts a fee, at most the cash the trader has. Returns the amount charged.
    double charge_fee(double amount);

    // Mark-to-market value and profit/loss, at a price in ticks.
    double equity(Price mark_price) const;
    double pnl(Price mark_price) const;

private:
    TraderId id_;

    double cash_;
    double reserved_cash_;

    Position position_;
    Quantity reserved_assets_;
    Quantity short_limit_;

    double initial_cash_;
    Position initial_position_;
};

}  // namespace market
