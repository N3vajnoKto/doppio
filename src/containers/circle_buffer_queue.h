#pragma once

#include <array>
#include <atomic>
#include <initializer_list>
#include <system_error>
#include <type_traits>

enum class CircleBufferErrorCode {
    Ok = 0,
    CircleBufferIsFull,
};

class CircleBufferErrorCategory : public std::error_category {
    const char* name() const noexcept override {
        return "circle_buffer_error_category";
    }

    std::string message(int ev) const override {
        switch (static_cast<CircleBufferErrorCode>(ev)) {
        case CircleBufferErrorCode::Ok:
            return "Ok";
        case CircleBufferErrorCode::CircleBufferIsFull:
            return "Circle buffer is full";
        }
        return "unknown error";
    }
};

const std::error_category& circle_buffer_category() {
    static CircleBufferErrorCategory instance;
    return instance;
}

std::error_code make_circle_buffer_error_code(CircleBufferErrorCode ec) {
    return {static_cast<int>(ec), circle_buffer_category()};
}

template <class T, size_t Size>
    requires ((Size & (Size - 1)) == 0 && std::is_nothrow_move_constructible_v<T>)
class CircleBufferQueue {
public:
    using EntryState = uint8_t;

    static constexpr EntryState kNoEntry = 0;
    static constexpr EntryState kEntryReady = 1;

    CircleBufferQueue()
        : head_(0)
        , tail_(0) {}

    CircleBufferQueue(std::initializer_list<T> list)
        : CircleBufferQueue() {
        for (auto& entry : list) {
            Push(std::move(entry));
        }
    }

    std::error_code Push(T&& entry) {
        if (tail_.load() - head_.load() == Size) {
            // error
        }
    }

private:
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
    std::array<std::atomic<EntryState>, Size> entry_states_;
    std::array<uint8_t, Size * sizeof(T)> data_;
};
