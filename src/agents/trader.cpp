#include "market/agents/trader.hpp"

#include <algorithm>

namespace market {

namespace {
// Tolerance for floating-point drift in cash bookkeeping.
constexpr double kEpsilon = 1e-9;
}

Trader::Trader(TraderId id, double cash, Quantity quantity)
    : id_(id),
      cash_(cash),
      reserved_cash_(0.0),
      asset_quantity_(quantity),
      reserved_assets_(0) {}

TraderId Trader::id() const { return id_; }

double Trader::cash() const { return cash_; }
double Trader::reserved_cash() const { return reserved_cash_; }
double Trader::available_cash() const { return cash_ - reserved_cash_; }

Quantity Trader::asset_quantity() const { return asset_quantity_; }
Quantity Trader::reserved_assets() const { return reserved_assets_; }
Quantity Trader::available_assets() const { return asset_quantity_ - reserved_assets_; }

void Trader::add_cash(double amount) {
    if (amount >= 0.0) { cash_ += amount; }
}

bool Trader::remove_cash(double amount) {
    if (amount < 0.0 || amount > available_cash()) { return false; }
    cash_ -= amount;
    return true;
}

void Trader::add_assets(Quantity quantity) {
    asset_quantity_ += quantity;
}

bool Trader::remove_assets(Quantity quantity) {
    if (quantity > available_assets()) { return false; }
    asset_quantity_ -= quantity;
    return true;
}

bool Trader::reserve_cash(double amount) {
    if (amount < 0.0 || amount > available_cash()) { return false; }
    reserved_cash_ += amount;
    return true;
}

bool Trader::release_cash(double amount) {
    if (amount < 0.0 || amount > reserved_cash_ + kEpsilon) { return false; }
    reserved_cash_ = std::max(0.0, reserved_cash_ - amount);
    return true;
}

bool Trader::reserve_assets(Quantity quantity) {
    if (quantity > available_assets()) { return false; }
    reserved_assets_ += quantity;
    return true;
}

bool Trader::release_assets(Quantity quantity) {
    if (quantity > reserved_assets_) { return false; }
    reserved_assets_ -= quantity;
    return true;
}

bool Trader::consume_reserved_cash(double amount) {
    if (amount < 0.0 || amount > reserved_cash_ + kEpsilon || amount > cash_ + kEpsilon) {
        return false;
    }
    reserved_cash_ = std::max(0.0, reserved_cash_ - amount);
    cash_ -= amount;
    return true;
}

bool Trader::consume_reserved_assets(Quantity quantity) {
    if (quantity > reserved_assets_ || quantity > asset_quantity_) { return false; }
    reserved_assets_ -= quantity;
    asset_quantity_ -= quantity;
    return true;
}

}  // namespace market
