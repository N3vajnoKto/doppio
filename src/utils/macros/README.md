# Error codes

Include `<utilcs/macros/error_code.h>` and invoke `CREATE_ERROR_CODE` at global
scope (outside any namespace):

```cpp
CREATE_ERROR_CODE(
    (doppio, containers),
    (ring_buffer, RingBufferError),
    (empty),
    (full, "Ring buffer is full"),
    (invalid, , std::make_error_code(std::errc::invalid_argument)),
)

std::error_code ec = doppio::containers::RingBufferError::empty;
auto full = doppio::containers::make_ring_buffer_error_code(
    doppio::containers::RingBufferError::full);
auto translated = doppio::containers::make_ring_buffer_error_code(
    std::make_error_code(std::errc::invalid_argument));
```

The name pair supplies the function prefix and enum type explicitly: the C++
preprocessor cannot convert arbitrary snake_case identifiers to PascalCase.
Namespace components are comma-separated; `()` selects the global namespace.
At least one variant is required. A trailing comma is supported.

Variants accept `(name)`, `(name, message)`, or `(name, message, source_code)`.
An omitted or empty message defaults to the variant identifier. Enum values start
at 1, so every declared variant represents an error. Casting 0 to the enum
produces a false error code, with an unspecified/unknown message.

The overload taking `std::error_code` compares against source codes in declaration
order and returns the first matching variant. Unknown codes, including a default
success code, are preserved. Source expressions are evaluated on each translation;
they should be side-effect-free. Expressions containing commas must be wrapped
in parentheses. Translation does not change `default_error_condition()` or make
codes from different categories equal.

The header requires C++20 or newer. Generated definitions are inline and may be
placed in a header shared by multiple translation units. Declare a given error
type only once per translation unit. Use distinct function prefixes within each
namespace.
