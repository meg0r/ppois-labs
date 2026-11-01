cmake_minimum_required(VERSION 3.16)
project(RepairCompanyLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Actors.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Services.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Warehouse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Orders.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Finance.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DiagnosticsQuality.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CLI.cpp
)

add_executable(repair_service_app ${CORE_SOURCES} src/main.cpp)

enable_testing()
add_subdirectory(tests)