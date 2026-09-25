#include <iostream>
#include <memory>

#include "market/core/config.hpp"
#include "market/market/market.hpp"
#include "market/models/fundamental_value.hpp"
#include "market/models/random_walk_model.hpp"
#include "market/simulation/simulation.hpp"

class TestAgent : public market::Agent {
	void step(market::Timestamp timestamp, market::Market& market) override {
		(void)market;

		std::cout
			<< "Agent activated at t = "
			<< timestamp
			<< '\n';
	}
};

int main() {
	market::SImulationConfig config;

	config.steps = 10;

	market::RandomWalkModel price_model(0.1, 1.0, 12345);
	market::FundamentalValue fundamental_value(100.0, price_model);
	
	market::Market market;
	market::Simulation simulation(config, market, fundamental_value);
	
	simulation.add_agent(std::make_unique<TestAgent>());
	simulation.run();

	std::cout
		<< "Final time: "
		<< simulation.current_time()
		<< '\n';

	std::cout
		<< "Final fundamental value: "
		<< simulation.fundamental_value()
		<< '\n';

	return 0;
}
	
