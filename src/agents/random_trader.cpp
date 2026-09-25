#include "market/agents/random_trader.hpp"

#include "market/agents/trader.hpp"
#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"
#include "market/market/order.hpp"

namespace market {
    	RandomTrader::RandomTrader(
		TraderId trader_id,
		Price reference_price,
		std::unint64_t seed
	) 
	      	: trader_id_(trader_id),
		reference_price_(reference_price),
		generator_(seed),
		side_distribution_(0, 1),
		quantity_distribution_(1, 10)
	{}
	
	void RandomTrader::step(
		Timestamp timestamp, 
		Market& market,
		TraderRegistry& traders,
		Settlement& settlement
	) {
        	Trader* trader = traders.find_trader(trader_id_);
		if (trader == nullptr) { return; }

		int side_value = side_distribution_(generator_);

		Quantity quantity = static_cast<Quantity>(quantity_distribution_(generator_));
		Side side = 
			side_value == 0
			? Side::Buy
			: Side::Sell;
		
		double order_value = reference_price_ * static_cast<double>(quantity);
		if (side == Side::Buy) {
			if (trader->cash() < order_value) { return; }
		} else {
			if (trader->asset_quantity() < quantity) { return; }
		}

		Order = order{
			.id = 0,
			.trader_id = trader->id,
			.side = side,
			.type = OrderType::Limit,
			.price = reference_price_,
			.quantity = quantity,
			.timestamp = timestamp
		};

		std::vector<Trader> traders = market.submit_order(order);

		for (const Trade& trade : trades) {
			settlement.settle(trade);
		}
	}
}
