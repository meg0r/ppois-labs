include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

set(TEST_SOURCES
    test_main.cpp
    test_actors.cpp
    test_diagnostics.cpp
    test_finance.cpp
    test_orders.cpp
    test_warehouse.cpp
)

add_executable(run_unit_tests ${TEST_SOURCES} ${CORE_SOURCES})
add_test(NAME DomainUnitTests COMMAND run_unit_tests)