#include "market/agents/trader.hpp"

namespace market {
    	Trader::Trader(
        	TraderId id,
        	double cash,
        	Quantity quantity
    	)
        	: id_(id),
          	cash_(cash),
		reserved_cash_(0.0),
          	asset_quantity_(quantity),
		reserved_assets_(0)
    	{}

    	TraderId Trader::id() const { 
		return id_; 
	}
    	
	double Trader::cash() const { 
		return cash_; 
	}
	
	double Trader::reserved_cash() const {
		return reserved_cash_;
	}

	double Trader::avaliable_cash() const {
		return cash_ - reserved_cash_;
	}

    	Quantity Trader::asset_quantity() const { 
		return asset_quantity_; 
	}

	Quantity Trader::reserved_quantity() const {
		return reserved_assets;
	}

	Quantity Trader::avaliable_assets() const {
		return asset_quantity_ - reserved_assets_;
	}

	bool Trader::reserve_cash(double amount) {
		if (amount < 0.0 || amount > avaliable_cash()) { return false; }

		reserve_cash_ += amount;
		return true;
	}

	bool Trader::release_cash(double amount) {
		if (amount < 0.0 || amount > avaliable_cash()) { return false; }

		reserved_cash_ -= amount;
		return true;
	}

	bool Trader::reserve_assets(Quantity quantity) {
		if (quantity > avaliable_assets()) { return false; }

		reserved_assets_ += quantity;
		return true;
	}

	bool Trader::release_assets(Quantity quantity) {
		if (quantity > reserved_assets()) { return false; }

		reserved_assets_ -= quantity;
		return true;
	}

    	void Trader::add_cash(double amount) { 
		if (amount >= 0.0) {
			cash_ += amount;
		}
	}

    	bool Trader::remove_cash(double amount) {
        	if (cash_ >= amount && amount >= 0.0) {
	           	cash_ -= amount;
        	    	return true;
        	}

        	return false;
    	}

    	void Trader::add_asset(Quantity quantity) { 
		asset_quantity_ += quantity; 
	}
	
	bool Trader::remove_asset(Quantity quantity) {
        	if (asset_quantity_ >= quantity && quantity >= 0) {
            		asset_quantity_ -= quantity;
            		return true;
        	}

       		return false;
   	}

	bool Trader::consume_reserved_cash(double cash) {
		if (amount < 0.0 || amount > reserved_cash_ || amount > cash_) { return false; }

		reserved_cash_ -= amount;
		cash_ -= amount;
		
		return true;
	}

	bool Trader::consume_reserved_assets(Quantity quantity) {
		if (quantity > reserved_assets_ || quantity > asset_quantity_) { return false; }

		reserved_assets_ -= quantity;
		asset_quantity_ -= quantity;

		return true;
	}
}
