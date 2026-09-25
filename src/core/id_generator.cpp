#include "market/core/id_generator.hpp"

namespace market {
	std::uint64_t IdGenerator::next() {
		return next_id_++;
	}
}
