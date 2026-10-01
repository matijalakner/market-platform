#include "market/simulation/event_scheduler.hpp"

#include <utility>

namespace market {

void EventScheduler::schedule(Timestamp time, Event event) {
    queue_.push(Item{time, next_sequence_++, std::move(event)});
}

void EventScheduler::run_until(Timestamp now) {
    while (!queue_.empty() && queue_.top().time <= now) {
        Item item = queue_.top();
        queue_.pop();
        item.event(item.time);
    }
}

std::size_t EventScheduler::pending() const { return queue_.size(); }

}  // namespace market
