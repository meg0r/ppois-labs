cmake_minimum_required(VERSION 3.14)
project(lab4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# Основное приложение (CLI)
add_executable(lab4_cli src/main.cpp)

# Подключение GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(lab4_tests tests/test_main.cpp)
target_link_libraries(lab4_tests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(lab4_tests)