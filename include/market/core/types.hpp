#pragma once
#include <cstdint>

// Common vocabulary for the project.

namespace market {
	
	using OrderId = std::uint64_t;
	using TraderId = std::uint64_t;
	using TimeStamp = std::uint64_t;

	using Price = double;
	using Quantity = std::uint64_t;

	enum class Side {
		Buy,
		Sell
	};

	enum class OrderType {
		Market,
		Limit
	};

}
