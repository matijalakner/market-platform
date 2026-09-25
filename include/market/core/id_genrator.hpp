#pragma once

#include <cstdint>

namespace market {
	class IdGenerator {
		public:
			std::uint64_t next();

		private:
			std::uint64_t next_id_ = 1;
	};
}
