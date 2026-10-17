#pragma once

#include <stdexcept>

namespace scheduler {

class TaskExecutionError : public std::runtime_error {
public:
    explicit TaskExecutionError(const char* msg)
        : std::runtime_error(msg) {}

    TaskExecutionError(TaskExecutionError&&) noexcept = default;
};

} // namespace scheduler
