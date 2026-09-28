#pragma once

#include "market/agents/agent.hpp"

namespace market {

	class FundamentalValue;

	class FundamentalTrader : public Agent {
		public:
    			FundamentalTrader(
				TraderId trader_id,
				Price threshold,
				Quantity order_quantity
			);

			void step(
				Timestamp timestamp,
				Market& market,
				TraderRegistry& traders,
				Settlement& settlement
			) override;

		private:
    			TraderId trader_id_;
			Price threshold_;
			Quantity order_quantity_;
	};
}
