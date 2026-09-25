#pragma once

#include "market/core/types.hpp"

namespace market {
	class Trader {
	public:
		Trader(
            		Trader_Id id,
            		double cash,
            		Quantity asset_quantity
        	);

		TraderId id() const;
		double cash() const;
        	Quantity asset_quantity() const;
        	void add_cash(double amount);
        	bool remove_cah(double amount);
        	void add_asset(Quantity quantity);
        	bool remove_asset(Quantity quantity);

    	private:
        	TraderId id_;
        	double cash_;
        	Quantity asset_quantity_;
	};
}
