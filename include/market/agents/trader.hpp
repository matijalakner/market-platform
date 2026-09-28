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
		double avaliable_cash() const;
		double reserved_cash() const;

        	Quantity asset_quantity() const;
		Quantity avaliable_assets() const;
		Quantity reserved_assets() const;

        	void add_cash(double amount);
        	bool remove_cah(double amount);

		bool reserve_cash(double amount);
		bool release_cash(double amount);

        	void add_asset(Quantity quantity);
        	bool remove_asset(Quantity quantity);
		
		bool reserve_assets(Quantity quantity);
		bool release_assets(Quantity quantity);
		
		bool consume_reserved_cash(double amount);
		bool consume_reserved_assets(Quantity quantity);

    	private:
        	TraderId id_;

        	double cash_;
		double reserved_cash_;
        	
		Quantity asset_quantity_;
		Quantity reserved_quantity_;
	};
}
