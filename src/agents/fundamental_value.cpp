#include "market/agents/fundamental_trader.hpp"

#include "market/agents/trader.hpp"
#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"
#include "market/market/order.hpp"
#include "market/market/settlement.hpp"
#include "market/models/fundamental_value.hpp"

namespace market {
	FundamentalTrader::FundamentalTrader(
		TraderId trader_id,
		Price threshold,
		Quantity order_quantity
	) 
		: trader_id_(trader_id),
		threshold_(threshold),
		order_quantity_(order_quantity)
	{}

	void FundamentalTrader::step(
		Timestamp timestamp,
		Market& market,
		TraderRegistry& traders,
		Settlement& settlement,
		FundamentalValue& fundamental_value
	) {
		Trader* trader = trader.find_trader(trader_id_);
		if (trader == nullptr) { return; }

		Price market_price;
		
		auto mid = market.mid_price();
		
		if (mid.has_value()) {
			market_price = mid.value();
		} else if (market.has_traded()) {
			market_price = market.last_trade_price();
		} else {
			market_price = fundamental_value.value();
		}

		Price fundamental = fundamental_value.value();
		
		Price mispricing = (fundamental - market_price) / market_price;

		if (mispricing > threshold_) {
			// BUY
		} else if (mispricing < -threshold) {
			// SELL
		} else {
			// HOLD
			return;
		}

		Price order_value = market_price * static_cast<double>(order_quantity_);
		if (side == Side::Buy) {
			if (trader->cash() < order_value) { return; }
		} else {
			if (trader->asset_quantity() < order_quantity_) { return; }
		}

		Order order {
			.id = 0,
			.trader_id = trader_id_,
			.side = side,
			.type = OrderType::Limit,
			.price = market_price,
			.quantity = order_quantity_,
			.timestamp = timestamp
		}

		auto trades = market.submit_order(order);

		for (const Trade& trade : trades) {
			settlement.settle(trade);
		}
	
	}
}
