#include <cassert>
#include <vector>

#include "market/simulation/event_scheduler.hpp"

int main() {
    market::EventScheduler scheduler;
    std::vector<int> order;

    scheduler.schedule(5, [&](market::Timestamp) { order.push_back(5); });
    scheduler.schedule(2, [&](market::Timestamp) { order.push_back(2); });
    scheduler.schedule(2, [&](market::Timestamp) { order.push_back(22); });  // same time: after the first
    scheduler.schedule(9, [&](market::Timestamp t) { order.push_back(static_cast<int>(t)); });

    assert(scheduler.pending() == 4);

    scheduler.run_until(1);
    assert(order.empty());

    scheduler.run_until(5);
    assert((order == std::vector<int>{2, 22, 5}));
    assert(scheduler.pending() == 1);

    scheduler.run_until(100);
    assert((order == std::vector<int>{2, 22, 5, 9}));
    assert(scheduler.pending() == 0);

    return 0;
}
