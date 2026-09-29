#pragma once

#include "market/core/types.hpp"

namespace market {

class Trader {
public:
    Trader(TraderId id, double cash, Quantity asset_quantity);

    TraderId id() const;

    double cash() const;
    double available_cash() const;
    double reserved_cash() const;

    Quantity asset_quantity() const;
    Quantity available_assets() const;
    Quantity reserved_assets() const;

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

private:
    TraderId id_;

    double cash_;
    double reserved_cash_;

    Quantity asset_quantity_;
    Quantity reserved_assets_;
};

}  // namespace market
