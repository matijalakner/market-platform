#pragma once

#include "market/agents/agent.hpp"

namespace market {
    	class RandomTrader : public Agent {
    	public:
    	    	explicit RandomTrader(
			TraderId trader_id,
			Price reference_price,
			std::unint64_t seed
		);
       		void step(
			Timestamp timestamp,
			Market& market,
			TraderRegistry& traders,
			Settlement& settlement
		) override;
        
    	private:
    		TraderId trader_id_;
		Price reference_price_;
		std::mt19937_64 generator_;
		std::uniform_int_distribution<int> side_distribution_;
		std::uniform_int_distribution<int> quantity_distribution_;
    	};
}
