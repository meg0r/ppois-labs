#include "Exceptions.hpp"
#include "Warehouse.hpp"

#include <cassert>
#include <iostream>

static void testStockAvailabilityTracking() {
    constexpr int PART_ID = 88;
    Warehouse warehouse(1, "Central Depot 1");
    StorageShelf shelf(1, "ROW-A1", 150.0);
    Supplier supplier(1, "PartSource", "+123456789", 4.5);
    SparePart part(PART_ID, "Display Module", "MOD-DISP", 75.0, &supplier);

    warehouse.registerShelf(&shelf);
    warehouse.receivePartToStock(&part, &shelf);
    assert(warehouse.checkStockAvailability(PART_ID));

    warehouse.issuePartFromStock(PART_ID);
    assert(!warehouse.checkStockAvailability(PART_ID));
}

static void testToolMaintenanceTrigger() {
    constexpr int TOOL_ID = 1;
    constexpr double INITIAL_WEAR = 70.0;
    constexpr double WEAR_INCREMENT = 15.0;

    Tool tool(TOOL_ID, "Soldering Station", INITIAL_WEAR, nullptr);
    assert(tool.isFitForOperation());

    tool.recordUsageWear(WEAR_INCREMENT);
    assert(!tool.isFitForOperation());
}

static void testShelfCapacityLimit() {
    StorageShelf shelf(1, "ROW-SMALL", 50.0);
    Supplier supplier(1, "FastShip", "+123456789", 4.0);
    SparePart part(1, "Small Screw", "SC-01", 1.0, &supplier);

    constexpr size_t CAPACITY_LIMIT = 30;
    for (size_t i = 0; i < CAPACITY_LIMIT; ++i) {
        shelf.placePart(&part);
    }
    bool exceptionThrown = false;
    try {
        shelf.placePart(&part);
    } catch (const RepairCompanyException&) {
        exceptionThrown = true;
    }
    assert(exceptionThrown);
}

void runWarehouseTests() {
    testStockAvailabilityTracking();
    testToolMaintenanceTrigger();
    testShelfCapacityLimit();
    std::cout << " [PASS] Warehouse module\n";
}