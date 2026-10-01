#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <queue>
#include <vector>

#include "market/core/types.hpp"

namespace market {

// Runs callbacks at scheduled simulation times, in time order
// (events at the same time run in the order they were scheduled).
class EventScheduler {
public:
    using Event = std::function<void(Timestamp)>;

    void schedule(Timestamp time, Event event);

    // Runs every event scheduled at or before `now`.
    void run_until(Timestamp now);

    std::size_t pending() const;

private:
    struct Item {
        Timestamp time;
        std::uint64_t sequence;
        Event event;
    };

    struct Later {
        bool operator()(const Item& a, const Item& b) const {
            if (a.time != b.time) { return a.time > b.time; }
            return a.sequence > b.sequence;
        }
    };

    std::priority_queue<Item, std::vector<Item>, Later> queue_;
    std::uint64_t next_sequence_ = 0;
};

}  // namespace market
