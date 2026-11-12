#include <gtest/gtest.h>
#include <utilcs/macros/error_code.h>

CREATE_ERROR_CODE(
    (doppio, test, errors),
    (ring_buffer, RingBufferError),
    (empty),
    (full, "Ring buffer is full"),
    (unavailable, "Unavailable", std::make_error_code(std::errc::resource_unavailable_try_again)),
    (invalid, , std::make_error_code(std::errc::invalid_argument)),
)

CREATE_ERROR_CODE((other), (buffer, BufferError), (empty))

TEST(ErrorCode, EnumAndMessages) {
    using namespace doppio::test::errors;
    const std::error_code empty = RingBufferError::empty;
    EXPECT_TRUE(empty);
    EXPECT_EQ(empty, make_ring_buffer_error_code(RingBufferError::empty));
    EXPECT_EQ(empty.message(), "empty");
    EXPECT_EQ(make_ring_buffer_error_code(RingBufferError::full).message(), "Ring buffer is full");
    EXPECT_STREQ(empty.category().name(), "doppio, test, errors:ring_buffer");
    EXPECT_EQ(std::error_code(999, empty.category()).message(), "Unknown error");
    EXPECT_NE(empty, other::make_buffer_error_code(other::BufferError::empty));
}

TEST(ErrorCode, TranslateExternalCodes) {
    using namespace doppio::test::errors;
    EXPECT_EQ(
        make_ring_buffer_error_code(std::make_error_code(std::errc::resource_unavailable_try_again)),
        RingBufferError::unavailable
    );
    const auto invalid = make_ring_buffer_error_code(std::make_error_code(std::errc::invalid_argument));
    EXPECT_EQ(invalid, RingBufferError::invalid);
    EXPECT_EQ(invalid.message(), "invalid");
    const auto unknown = std::make_error_code(std::errc::permission_denied);
    EXPECT_EQ(make_ring_buffer_error_code(unknown), unknown);
    EXPECT_FALSE(make_ring_buffer_error_code(std::error_code{}));
}
