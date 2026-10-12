#include "Actors.hpp"
#include "DiagnosticsQuality.hpp"
#include "Exceptions.hpp"
#include "Orders.hpp"

#include <cassert>
#include <iostream>

static void testShiftDurationLimit() {
    constexpr int SHIFT_ID = 1;
    constexpr int INVALID_HOURS = 14;
    bool exceptionThrown = false;
    try {
        WorkShift shift(SHIFT_ID, "SHIFT-NIGHT", INVALID_HOURS);
    } catch (const RepairCompanyException&) {
        exceptionThrown = true;
    }
    assert(exceptionThrown);
}

static void testQualityInspectionFailure() {
    constexpr int FAILING_SCORE = 60;
    IndividualClient client(1, "Mark", "+123456789", "2026-01-01", "ID-2", "1988-01-01");
    RepairRequest request(1, "Battery fix", "2026-01-01", RequestPriority::HIGH, &client, nullptr);
    Technician inspector(1, "Inspector", 900.0, 5);
    WorkOrder order(1, "2026-01-02", &request, &inspector);
    QualityInspection inspection(1, &inspector, &order);

    bool exceptionThrown = false;
    try {
        inspection.evaluateQuality(FAILING_SCORE, "Contact loose");
    } catch (const QualityCheckFailedException&) {
        exceptionThrown = true;
    }
    assert(exceptionThrown);
    assert(!inspection.hasPassedInspection());
}

static void testDeviceAccuracyCalibration() {
    constexpr double INITIAL_ACCURACY = 96.0;
    constexpr double DEGRADATION = 8.0;
    DiagnosticDevice device(1, "SN-0091", "Oscilloscope", INITIAL_ACCURACY);
    device.degradeAccuracy(DEGRADATION);
    assert(!device.isDeviceOperational());

    device.calibrateDevice();
    assert(device.isDeviceOperational());
}

void runDiagnosticsTests() {
    testShiftDurationLimit();
    testQualityInspectionFailure();
    testDeviceAccuracyCalibration();
    std::cout << " [PASS] Diagnostics module\n";
}