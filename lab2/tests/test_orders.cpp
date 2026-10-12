#include "Actors.hpp"
#include "Exceptions.hpp"
#include "Orders.hpp"
#include "Warehouse.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

static void testRequestShortDescription() {
    IndividualClient client(1, "George", "+123456789", "2026-01-01", "ID-10", "1991-01-01");
    bool exceptionThrown = false;
    try {
        RepairRequest request(1, "Bad", "2026-01-01", RequestPriority::LOW, &client, nullptr);
    } catch (const RepairCompanyException&) {
        exceptionThrown = true;
    }
    assert(exceptionThrown);
}

static void testWorkOrderImmutability() {
    IndividualClient client(1, "George", "+123456789", "2026-01-01", "ID-10", "1991-01-01");
    RepairRequest request(1, "Board repair", "2026-01-01", RequestPriority::HIGH, &client, nullptr);
    Technician tech(1, "Tech Lead", 850.0, 4);
    WorkOrder order(1, "2026-01-02", &request, &tech);

    order.startExecution();
    order.completeExecution("2026-01-04");
    assert(order.isClosed());

    bool exceptionThrown = false;
    try {
        order.completeExecution("2026-01-05");
    } catch (const OrderAlreadyCompletedException&) {
        exceptionThrown = true;
    }
    assert(exceptionThrown);
}

static void testPurchaseOrderCostCalculation() {
    constexpr int QUANTITY = 8;
    constexpr double UNIT_PRICE = 15.0;
    constexpr double EXPECTED = 120.0;
    constexpr double EPSILON = 0.001;

    Supplier supplier(1, "MicroParts LLC", "+123456789", 4.8);
    SparePart part(1, "Capacitor 100uF", "CAP-100", UNIT_PRICE, &supplier);
    PurchaseOrder po(1, QUANTITY, UNIT_PRICE, &supplier, &part);
    assert(std::abs(po.calculateTotalCost() - EXPECTED) < EPSILON);
}

void runOrdersTests() {
    testRequestShortDescription();
    testWorkOrderImmutability();
    testPurchaseOrderCostCalculation();
    std::cout << " [PASS] Orders module\n";
}