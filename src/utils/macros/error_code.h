#pragma once

#include <string>
#include <system_error>
#include <type_traits>

// Invoke at global scope. Names are (function_prefix, EnumType).
// Variants are (name), (name, message), or (name, message, source_error_code).
// Use (name, , source_error_code) to keep the default message.
#define DOPPIO_EC_CAT_I(a, b) a##b
#define DOPPIO_EC_CAT(a, b) DOPPIO_EC_CAT_I(a, b)
#define DOPPIO_EC_UNPACK(...) __VA_ARGS__
#define DOPPIO_EC_FIRST(a, ...) a
#define DOPPIO_EC_SECOND(a, b, ...) b
#define DOPPIO_EC_PREFIX(names) DOPPIO_EC_FIRST names
#define DOPPIO_EC_TYPE(names) DOPPIO_EC_SECOND names
#define DOPPIO_EC_FUNCTION(names) DOPPIO_EC_CAT(DOPPIO_EC_CAT(make_, DOPPIO_EC_PREFIX(names)), _error_code)
#define DOPPIO_EC_CATEGORY(names) DOPPIO_EC_CAT(DOPPIO_EC_PREFIX(names), _error_category)
#define DOPPIO_EC_STRING_I(...) #__VA_ARGS__
#define DOPPIO_EC_STRING(...) DOPPIO_EC_STRING_I(__VA_ARGS__)

// Deferred expansion supports variable-length lists and a trailing comma.
#define DOPPIO_EC_EXPAND1(...) __VA_ARGS__
#define DOPPIO_EC_EXPAND2(...) DOPPIO_EC_EXPAND1(DOPPIO_EC_EXPAND1(DOPPIO_EC_EXPAND1(__VA_ARGS__)))
#define DOPPIO_EC_EXPAND3(...) DOPPIO_EC_EXPAND2(DOPPIO_EC_EXPAND2(DOPPIO_EC_EXPAND2(__VA_ARGS__)))
#define DOPPIO_EC_EXPAND(...) DOPPIO_EC_EXPAND3(DOPPIO_EC_EXPAND3(DOPPIO_EC_EXPAND3(__VA_ARGS__)))
#define DOPPIO_EC_PARENS ()
#define DOPPIO_EC_EACH(m, d, ...) __VA_OPT__(DOPPIO_EC_EXPAND(DOPPIO_EC_EACH_I(m, d, __VA_ARGS__)))
#define DOPPIO_EC_EACH_I(m, d, a, ...) m(d, a) __VA_OPT__(DOPPIO_EC_AGAIN DOPPIO_EC_PARENS(m, d, __VA_ARGS__))
#define DOPPIO_EC_AGAIN() DOPPIO_EC_EACH_I

#define DOPPIO_EC_NS_OPEN(d, n) namespace n {
#define DOPPIO_EC_NS_CLOSE(d, n) }
#define DOPPIO_EC_NS_PATH(d, n) n::
#define DOPPIO_EC_ENUM(d, item) DOPPIO_EC_FIRST item,
#define DOPPIO_EC_ENUMS(first, ...) DOPPIO_EC_FIRST first = 1, DOPPIO_EC_EACH(DOPPIO_EC_ENUM, _, __VA_ARGS__)
#define DOPPIO_EC_MESSAGE(d, item) DOPPIO_EC_MESSAGE_I item
#define DOPPIO_EC_MESSAGE_I(name, ...) DOPPIO_EC_MESSAGE_II(name, __VA_ARGS__, , )
#define DOPPIO_EC_MESSAGE_II(name, message, ...)                                                                       \
    case static_cast<int>(Error::name):                                                                                \
        return DOPPIO_EC_TEXT(name, message);
#define DOPPIO_EC_TEXT(name, ...) DOPPIO_EC_TEXT_I(name __VA_OPT__(, __VA_ARGS__), DOPPIO_EC_STRING(name))
#define DOPPIO_EC_TEXT_I(name, text, ...) text
#define DOPPIO_EC_TRANSLATE(d, item) DOPPIO_EC_TRANSLATE_I item
#define DOPPIO_EC_TRANSLATE_I(name, ...) DOPPIO_EC_TRANSLATE_II(name, __VA_ARGS__, , )
#define DOPPIO_EC_TRANSLATE_II(name, message, source, ...) DOPPIO_EC_TRANSLATE_IV(name, source)
#define DOPPIO_EC_TRANSLATE_IV(name, ...)                                                                              \
    __VA_OPT__(if (source == (__VA_ARGS__)) { return make_error_code(Error::name); })

#define CREATE_ERROR_CODE(path, names, ...)                                                                            \
    DOPPIO_EC_EACH(DOPPIO_EC_NS_OPEN, _, DOPPIO_EC_UNPACK path)                                                        \
    enum class DOPPIO_EC_TYPE(names) {                                                                                 \
        DOPPIO_EC_ENUMS(__VA_ARGS__)                                                                                   \
    };                                                                                                                 \
    inline const std::error_category& DOPPIO_EC_CATEGORY(names)() noexcept {                                           \
        class Category final : public std::error_category {                                                            \
            using Error [[maybe_unused]] = DOPPIO_EC_TYPE(names);                                                      \
                                                                                                                       \
        public:                                                                                                        \
            const char* name() const noexcept override {                                                               \
                return DOPPIO_EC_STRING(DOPPIO_EC_UNPACK path) ":" DOPPIO_EC_STRING(DOPPIO_EC_PREFIX(names));          \
            }                                                                                                          \
            std::string message(int value) const override {                                                            \
                switch (value) { DOPPIO_EC_EACH(DOPPIO_EC_MESSAGE, _, __VA_ARGS__) }                                   \
                return "Unknown error";                                                                                \
            }                                                                                                          \
        };                                                                                                             \
        static const Category category;                                                                                \
        return category;                                                                                               \
    }                                                                                                                  \
    inline std::error_code make_error_code(DOPPIO_EC_TYPE(names) value) noexcept {                                     \
        return {static_cast<int>(value), DOPPIO_EC_CATEGORY(names)()};                                                 \
    }                                                                                                                  \
    inline std::error_code DOPPIO_EC_FUNCTION(names)(DOPPIO_EC_TYPE(names) value) noexcept {                           \
        return make_error_code(value);                                                                                 \
    }                                                                                                                  \
    inline std::error_code DOPPIO_EC_FUNCTION(names)(const std::error_code& source) {                                  \
        using Error [[maybe_unused]] = DOPPIO_EC_TYPE(names);                                                          \
        DOPPIO_EC_EACH(DOPPIO_EC_TRANSLATE, _, __VA_ARGS__)                                                            \
        return source;                                                                                                 \
    }                                                                                                                  \
    DOPPIO_EC_EACH(DOPPIO_EC_NS_CLOSE, _, DOPPIO_EC_UNPACK path)                                                       \
    template <>                                                                                                        \
    struct std::is_error_code_enum<DOPPIO_EC_EACH(DOPPIO_EC_NS_PATH, _, DOPPIO_EC_UNPACK path) DOPPIO_EC_TYPE(names)>  \
        : std::true_type {};
