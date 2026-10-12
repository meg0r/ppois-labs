#include "Actors.hpp"
#include "Exceptions.hpp"
#include "Orders.hpp"

#include <cassert>
#include <iostream>

static void testClientPhoneValidation() {
    constexpr int CLIENT_ID = 101;
    const std::string SHORT_PHONE = "12345";
    bool exceptionThrown = false;
    try {
        IndividualClient client(CLIENT_ID, "Alice", SHORT_PHONE, "2026-01-01", "ID-11", "1990-01-01");
    } catch (const RepairCompanyException&) {
        exceptionThrown = true;
    }
    assert(exceptionThrown);
}

static void testEmployeeSalaryThreshold() {
    constexpr int EMP_ID = 201;
    constexpr double INVALID_SALARY = 450.0;
    bool exceptionThrown = false;
    try {
        Technician tech(EMP_ID, "Bob", INVALID_SALARY, 2);
    } catch (const RepairCompanyException&) {
        exceptionThrown = true;
    }
    assert(exceptionThrown);
}

static void testTechnicianBusyState() {
    constexpr double SALARY = 800.0;
    constexpr int GRADE = 3;
    IndividualClient client(1, "John", "+123456789", "2026-01-01", "ID-1", "1992-01-01");
    RepairRequest request(1, "Fault fix", "2026-01-01", RequestPriority::HIGH, &client, nullptr);
    Technician tech(1, "Dave", SALARY, GRADE);
    WorkOrder orderA(1, "2026-01-02", &request, &tech);
    WorkOrder orderB(2, "2026-01-02", &request, &tech);

    tech.assignWorkOrder(&orderA);
    bool exceptionThrown = false;
    try {
        tech.assignWorkOrder(&orderB);
    } catch (const TechnicianBusyException&) {
        exceptionThrown = true;
    }
    assert(exceptionThrown);
}

void runActorsTests() {
    testClientPhoneValidation();
    testEmployeeSalaryThreshold();
    testTechnicianBusyState();
    std::cout << " [PASS] Actors module\n";
}