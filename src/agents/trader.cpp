#include "market/agents/trader.hpp"

namespace market {
    Trader::Trader(
        TraderId id,
        double cash,
        Quantity quantity
    )
        : id_(id),
          cash_(cash),
          asset_quantity_(quantity)
    {}

    TraderId Trader::id() const { return id_; }
    double Trader::cash() const { return cash_; }
    Quantity Trader::asset_quantity() const { return asset_quantity_; }
    void Trader::add_cash(double amount) { cash_ += amount; }
    bool Trader::remove_cash(double amount) {
        if (cash_ >= amount) {
            cash_ -= amount;
            return true;
        };
        return false;
    }
    void Trader::add_asset(Quantity quantity) { asset_quantity_ += quantity; }
    bool Trader::remove_asset(Quantity quantity) {
        if (asset_quantity_ >= quantity) {
            asset_quantity_ -= quantity;
            return true;
        }
        return false;
    }
}