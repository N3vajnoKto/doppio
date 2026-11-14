#include <gtest/gtest.h>
#include <scheduler/except.h>
#include <scheduler/task.h>

#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

// Reference parameters are safe here: tasks are resumed synchronously in the caller's scope.
doppio::Task<void> addCounter(int& counter) {
    counter++;
    co_return;
}

doppio::Task<void> loopCounter(int& counter, int n) {
    // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores): the loop index is intentionally unused
    for (int i : std::ranges::iota_view(0, n)) {
        co_await addCounter(counter);
    }
}

doppio::Task<int> recurseCount(int depth) {
    if (depth == 0) {
        co_return 0;
    }
    int res = co_await recurseCount(depth - 1);
    co_return res + depth;
}

} // namespace

TEST(Task, JustWorks) {
    int counter = 0;

    auto c = addCounter(counter);
    c();
    ASSERT_EQ(counter, 1);
}

TEST(Task, TasksLoop) {
    int counter = 0;

    const int loop_size = 5;

    auto c = loopCounter(counter, loop_size);
    c();
    ASSERT_EQ(counter, loop_size);
}

TEST(Task, RecurseTask) {
    int counter = 0;

    const int depth = 5;

    auto sum = recurseCount(depth)();
    ASSERT_EQ(depth * (depth + 1) / 2, sum);
}

namespace {

doppio::Task<void> throwException(const std::string& message) { throw std::runtime_error(message); }

doppio::Task<void> callThrowable(const std::string& message) { co_await throwException(message); }

doppio::Task<bool> catchException(const std::string& message) {
    try {
        co_await throwException(message);
        co_return false;
    } catch (std::runtime_error& e) {
        co_return e.what() == message;
    }
}

} // namespace

TEST(Task, Exception) {
    const std::string msg = "exception thrown";
    ASSERT_THROW(callThrowable(msg)(), std::runtime_error);
    ASSERT_TRUE(catchException(msg)());
}

namespace {

doppio::Task<void> loopCounterWithException(int& counter, int& except_counter, int n) {
    // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores): the loop index is intentionally unused
    for (int i : std::ranges::iota_view(0, n)) {
        co_await addCounter(counter);
        try {
            co_await throwException("error");
        } catch (...) {
            ++except_counter;
        }
    }
    throw std::runtime_error("end_of_loop");
}

} // namespace

TEST(Task, Stress) {
    const int iterations = 1'000'000;
    int counter = 0;
    int except_counter = 0;
    auto task = loopCounterWithException(counter, except_counter, iterations);
    ASSERT_THROW(task(), std::runtime_error);
    EXPECT_EQ(counter, iterations);
    EXPECT_EQ(except_counter, iterations);
}

namespace {

doppio::Task<int> dummy(int result) { co_return result; }

} // namespace

TEST(Task, Move) {
    const int mem = 1337;
    {
        auto a = dummy(mem);
        auto b = std::move(a);
        auto c = dummy(0);
        c = std::move(b);
        ASSERT_EQ(c(), mem);
        // NOLINTNEXTLINE(bugprone-use-after-move): the test checks that a moved-from task throws
        ASSERT_ANY_THROW(b());
    }
    {
        std::vector<doppio::Task<int>> a;

        a.push_back(dummy(1));
        a.push_back(dummy(2));
        a.push_back(dummy(3));

        EXPECT_EQ(a.at(0)(), 1);
        EXPECT_EQ(a.at(1)(), 2);
        EXPECT_EQ(a.at(2)(), 3);
    }
}

namespace {

doppio::Task<void> callDeletedTask() {
    auto t = dummy(1);
    [[maybe_unused]] auto moved = std::move(t);
    // NOLINTNEXTLINE(bugprone-use-after-move): the test checks that awaiting a moved-from task throws
    co_await t;
}

} // namespace

TEST(Task, DeletedTask) { ASSERT_THROW(callDeletedTask()(), doppio::TaskDeletedError); }
