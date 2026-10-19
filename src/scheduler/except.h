#pragma once

#include <stdexcept>

namespace doppio {

class TaskExecutionError : public std::runtime_error {
public:
    explicit TaskExecutionError(const char* msg)
        : std::runtime_error(msg) {}

    TaskExecutionError(TaskExecutionError&&) noexcept = default;
};

class TaskDeletedError : public std::runtime_error {
public:
    explicit TaskDeletedError(const char* msg = "Task coroutine is nullptr")
        : std::runtime_error(msg) {}

    TaskDeletedError(TaskDeletedError&&) noexcept = default;
};

} // namespace doppio
