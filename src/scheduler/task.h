#pragma once

#include <scheduler/except.h>

#include <concepts>
#include <coroutine>
#include <type_traits>
#include <utility>

namespace doppio {

template <class T = void>
class [[nodiscard]] Task {
    class FinalAwaiter;

    struct Promise {
        std::coroutine_handle<> continuation = std::noop_coroutine();
        std::exception_ptr exception_ptr = nullptr;
        T result;

        Task get_return_object() noexcept {
            return Task(std::coroutine_handle<Promise>::from_promise(*this));
        }

        void unhandled_exception() noexcept {
            exception_ptr = std::current_exception();
        }

        auto initial_suspend() noexcept {
            return std::suspend_always();
        }

        auto final_suspend() noexcept {
            return FinalAwaiter();
        }

        template <std::convertible_to<T> S>
        void return_value(S&& value) {
            result = std::forward<S>(value);
        }
    };

    class Awaiter {
    public:
        explicit Awaiter(std::coroutine_handle<Promise> handle) noexcept
            : handle_(handle) {}

        bool await_ready() noexcept {
            return !handle_ || handle_.done();
        }

        std::coroutine_handle<> await_suspend(std::coroutine_handle<> calling) noexcept {
            handle_.promise().continuation = calling;
            return handle_;
        }

        template <class S = T>
            requires (!std::is_same_v<void, S>)
        auto await_resume() {
            if (auto ptr = handle_.promise().exception_ptr; ptr) {
                std::rethrow_exception(ptr);
            }
            return std::move(handle_.promise().result);
        }

        template <class S = T>
            requires (std::is_same_v<void, S>)
        void await_resume() {
            if (auto ptr = handle_.promise().exception_ptr; ptr) {
                std::rethrow_exception(ptr);
            }
        }

    private:
        std::coroutine_handle<Promise> handle_;
    };

    class FinalAwaiter {
    public:
        bool await_ready() noexcept {
            return false;
        }

        auto await_suspend(std::coroutine_handle<Promise> self) noexcept {
            return self.promise().continuation;
        }

        void await_resume() noexcept {}
    };

public:
    using promise_type = Promise;

    explicit Task(std::coroutine_handle<Promise> self) noexcept
        : handle_(self) {}

    ~Task() {
        if (handle_) {
            handle_.destroy();
        }
    }

    Task(const Task&) = delete;

    Task& operator=(const Task&) = delete;

    auto operator co_await() noexcept {
        return Awaiter(handle_);
    }

    template <class S = T>
        requires (!std::is_same_v<void, S>)
    S operator()() {
        handle_.resume();
        if (auto ptr = handle_.promise().exception_ptr; ptr) {
            std::rethrow_exception(ptr);
        }
        return handle_.promise().result;
    }

    template <class S = T>
        requires (std::is_same_v<void, S>)
    void operator()() {
        handle_.resume();
        if (auto ptr = handle_.promise().exception_ptr; ptr) {
            std::rethrow_exception(ptr);
        }
    }

private:
    std::coroutine_handle<Promise> handle_;
};

template <>
struct Task<void>::Promise {
    std::coroutine_handle<> continuation = std::noop_coroutine();
    std::exception_ptr exception_ptr = nullptr;

    auto get_return_object() {
        return Task(std::coroutine_handle<Promise>::from_promise(*this));
    }

    void unhandled_exception() noexcept {
        exception_ptr = std::current_exception();
    }

    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return FinalAwaiter();
    }

    void return_void() {}
};

} // namespace doppio
