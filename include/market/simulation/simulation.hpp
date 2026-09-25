#pragma once

#include "market/core/config.hpp"
#include "market/core/types.hpp"
#include "market/models/fundamental_value.hpp"
#include "market/market/market.hpp"
#include "market/agents/trader_registry.hpp"

namespace market {
    	class Simulation {
    	public:
        	explicit Simulation(
            		const SimulationConfig& config,
            		Market& market,
            		FundamentalValue& fundamental_value,
        		TraderRegistry& traders
		);
        	
		void run();
        	Timestamp current_time() const;
        	Price fundamental_value() const;

    	private:
        	SimulationConfig config_;
        	Timestamp current_time_;
        	Market& market_;
        	FundamentalValue& fundamental_value_;
		TraderRegistry& traders;

		void step();
	};
}
