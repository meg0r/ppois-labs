cmake_minimum_required(VERSION 3.15)
project(FIRST_LAB_PPOIS_RUBICK CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(UTPP_BUILD_TESTS OFF CACHE BOOL "Disable internal tests" FORCE)
add_subdirectory(unittest-cpp)

add_library(rubik_core STATIC
    src/RubikCube.cpp
)

target_include_directories(rubik_core PUBLIC
    src/include
)

add_executable(FIRST_LAB_PPOIS_RUBICK
    src/main.cpp
)

target_link_libraries(FIRST_LAB_PPOIS_RUBICK PRIVATE
    rubik_core
)

enable_testing()

add_executable(rubik_tests
    tests/test_main.cpp
)

target_link_libraries(rubik_tests PRIVATE
    rubik_core
    UnitTest++
)

target_include_directories(rubik_tests PRIVATE
    unittest-cpp/UnitTest++
)

add_test(NAME AllTests COMMAND rubik_tests)