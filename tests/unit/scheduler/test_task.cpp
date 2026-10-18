#include <gtest/gtest.h>
#include <scheduler/task.h>

#include <ranges>
#include <stdexcept>

using namespace doppio;

namespace {

Task<void> AddCounter(int& counter) {
    counter++;
    co_return;
}

Task<void> LoopCounter(int& counter, int n) {
    for (int _ : std::ranges::iota_view(0, n)) {
        co_await AddCounter(counter);
    }
}

Task<int> RecurseCount(int depth) {
    if (depth == 0) {
        co_return 0;
    }
    int res = co_await RecurseCount(depth - 1);
    co_return res + depth;
}

} // namespace

TEST(Task, JustWorks) {
    int counter = 0;

    auto c = AddCounter(counter);
    c();
    ASSERT_EQ(counter, 1);
}

TEST(Task, TasksLoop) {
    int counter = 0;

    const int loop_size = 5;

    auto c = LoopCounter(counter, loop_size);
    c();
    ASSERT_EQ(counter, loop_size);
}

TEST(Task, RecurseTask) {
    int counter = 0;

    const int depth = 5;

    auto sum = RecurseCount(depth)();
    ASSERT_EQ(depth * (depth + 1) / 2, sum);
}

namespace {

Task<void> ThrowException(const std::string& message) {
    throw std::runtime_error(message);
}

Task<void> CallThrowable(const std::string& message) {
    co_await ThrowException(message);
}

Task<bool> CatchException(const std::string& message) {
    try {
        co_await ThrowException(message);
        co_return false;
    } catch (std::runtime_error& e) {
        co_return e.what() == message;
    }
}

} // namespace

TEST(Task, Exception) {
    const std::string msg = "exception thrown";
    ASSERT_THROW(CallThrowable(msg)(), std::runtime_error);
    ASSERT_TRUE(CatchException(msg)());
}

namespace {

Task<void> LoopCounterWithException(int& counter, int& except_counter, int n) {
    for (int _ : std::ranges::iota_view(0, n)) {
        co_await AddCounter(counter);
        try {
            co_await ThrowException("error");
        } catch (...) {
            ++except_counter;
        }
    }
    throw std::runtime_error("end_of_loop");
}

} // namespace

TEST(Task, Stress) {
    const int N = 1'000'000;
    int counter = 0;
    int except_counter = 0;
    auto task = LoopCounterWithException(counter, except_counter, N);
    ASSERT_THROW(task(), std::runtime_error);
    EXPECT_EQ(counter, N);
    EXPECT_EQ(except_counter, N);
}
