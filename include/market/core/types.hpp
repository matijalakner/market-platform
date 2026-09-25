#pragma once
#include <cstdint>

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
