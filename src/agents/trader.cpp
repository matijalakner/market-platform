#include "market/agents/trader.hpp"

#include <algorithm>

namespace market {

namespace {
// Tolerance for floating-point drift in cash bookkeeping.
constexpr double kEpsilon = 1e-9;
}

Trader::Trader(TraderId id, double cash, Position position, Quantity short_limit)
    : id_(id),
      cash_(cash),
      reserved_cash_(0.0),
      position_(position),
      reserved_assets_(0),
      short_limit_(short_limit),
      initial_cash_(cash),
      initial_position_(position) {}

TraderId Trader::id() const { return id_; }

double Trader::cash() const { return cash_; }
double Trader::reserved_cash() const { return reserved_cash_; }
double Trader::available_cash() const { return cash_ - reserved_cash_; }

Position Trader::asset_quantity() const { return position_; }
Quantity Trader::reserved_assets() const { return reserved_assets_; }
Quantity Trader::short_limit() const { return short_limit_; }

Quantity Trader::available_assets() const {
    Position headroom = position_
                      + static_cast<Position>(short_limit_)
                      - static_cast<Position>(reserved_assets_);
    return headroom > 0 ? static_cast<Quantity>(headroom) : 0;
}

void Trader::add_cash(double amount) {
    if (amount >= 0.0) { cash_ += amount; }
}

bool Trader::remove_cash(double amount) {
    if (amount < 0.0 || amount > available_cash()) { return false; }
    cash_ -= amount;
    return true;
}

void Trader::add_assets(Quantity quantity) {
    position_ += static_cast<Position>(quantity);
}

bool Trader::remove_assets(Quantity quantity) {
    if (quantity > available_assets()) { return false; }
    position_ -= static_cast<Position>(quantity);
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
    if (quantity > reserved_assets_) { return false; }
    reserved_assets_ -= quantity;
    position_ -= static_cast<Position>(quantity);  // may go negative (short)
    return true;
}

double Trader::charge_fee(double amount) {
    if (amount <= 0.0) { return 0.0; }
    double charged = std::min(amount, std::max(0.0, cash_));
    cash_ -= charged;
    return charged;
}

double Trader::equity(Price mark_price) const {
    return cash_ + static_cast<double>(position_) * static_cast<double>(mark_price);
}

double Trader::pnl(Price mark_price) const {
    double initial = initial_cash_ + static_cast<double>(initial_position_) * static_cast<double>(mark_price);
    return equity(mark_price) - initial;
}

}  // namespace market
