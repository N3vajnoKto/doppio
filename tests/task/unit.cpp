#include <gtest/gtest.h>
#include <scheduler/task.h>

#include <ranges>

namespace {

int counter = 0;

doppio::Task<void> f() {
    counter++;
    co_return;
}

doppio::Task<void> loop(int n) {
    for (int _ : std::ranges::iota_view(0, n)) {
        co_await f();
    }
}
} // namespace

TEST(Task, JustWorks) {
    const int N = 1'000;
    auto task = loop(N);
    task();
    ASSERT_EQ(counter, N);
}
