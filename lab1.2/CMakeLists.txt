cmake_minimum_required(VERSION 3.15)
project(TuringMachineSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/W4 /permissive- /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(turing_core STATIC
    src/Alphabet.cpp
    src/Head.cpp
    src/Tape.cpp
    src/Program.cpp
    src/TuringMachine.cpp
)

target_include_directories(turing_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(tests_runner tests/test_turing.cpp)
target_link_libraries(tests_runner PRIVATE turing_core)

add_executable(turing_app src/main.cpp)
target_link_libraries(turing_app PRIVATE turing_core)
