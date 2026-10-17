cmake_minimum_required(VERSION 3.30)

project(scheduler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)
enable_testing()

file(GLOB tests_src tests/*/*.cpp)

add_executable(tests ${tests_src})

target_link_libraries(tests PRIVATE gtest_main gmock)
target_include_directories(tests PUBLIC src)

gtest_discover_tests(tests)