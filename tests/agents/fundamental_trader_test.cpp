#include <cassert>

#include "market/agents/fundamental_trader.hpp"
#include "market/agents/trader_registry.hpp"
#include "market/market/market.hpp"
#include "market/market/settlement.hpp"
#include "market/models/fundamental_value.hpp"
#include "market/models/price_model.hpp"

class ConstantPriceModel : public market::PriceModel {
	public:
		market::Price next_price(market::PRice current_price) override
		{
			return current_price;
		}
};

int main() {
	ConstantPriceModel model;

	market::FundamentalValue fundamental(110.0, model);
	market::Trader trader(1, 10000.0, 100);

	market::TraderRegistry traders;

	traders.add_trader(trader);

	market::Market market;
	market::Settlement settlement(traders);

	market::FundamentalTrader fundamental_trader(1, 0.02, 10);
	fundamental_trader.step(
		1,
		market,
		traders,
		settlement,
		fundamental
	);

	// There was no existingg ask order, so the trader's buy order should now be resting in the book.
	
	assert(market.best_bid().has_value());
	assert(market.best_bid().value() == 0);

	return 0;
}
