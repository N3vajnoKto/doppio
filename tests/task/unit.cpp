#include <gtest/gtest.h>
#include <scheduler/task.h>

#include <ranges>

int counter = 0;

scheduler::Task<void> f() {
    counter++;
    co_return;
}

scheduler::Task<void> loop(int n) {
    for (int i : std::ranges::iota_view(0, n)) {
        co_await f();
    }
}

TEST(Task, JustWorks) {
    const int N = 1'000'000;
    auto task = loop(N);
    task();
    ASSERT_EQ(counter, N);
}
