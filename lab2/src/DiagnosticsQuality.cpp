#include "DiagnosticsQuality.hpp"
#include "Exceptions.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

// ==========================================
// WorkShift
// ==========================================

WorkShift::WorkShift(int id, std::string code, int durationHours)
    : shiftId(id),
      shiftCode(std::move(code)),
      plannedDurationHours(durationHours),
      isShiftActive(false) {
    if (shiftCode.empty()) {
        throw RepairCompanyException("Work shift code cannot be empty.");
    }
    if (durationHours <= 0 || durationHours > MAXIMUM_PERMISSIBLE_SHIFT_HOURS) {
        throw RepairCompanyException("Shift duration exceeds permissible operational limit.");
    }
}

int WorkShift::getShiftId() const noexcept {
    return shiftId;
}

void WorkShift::openShift() {
    if (isShiftActive) {
        throw RepairCompanyException("This work shift has already been opened.");
    }
    isShiftActive = true;
}

void WorkShift::closeShift() {
    if (!isShiftActive) {
        throw RepairCompanyException("Cannot close an inactive work shift.");
    }
    isShiftActive = false;
}

bool WorkShift::isActive() const noexcept {
    return isShiftActive;
}

// ==========================================
// RepairMetric
// ==========================================

RepairMetric::RepairMetric(int id, std::string name, double target)
    : metricId(id),
      metricName(std::move(name)),
      targetScore(target),
      actualScore(0.0) {
    if (metricName.empty()) {
        throw RepairCompanyException("Performance metric name cannot be empty.");
    }
    if (target <= 0.0) {
        throw RepairCompanyException("Target score must be strictly positive.");
    }
}

void RepairMetric::recordActualScore(double recordedValue) {
    if (recordedValue < 0.0) {
        throw RepairCompanyException("Actual metric score cannot be negative.");
    }
    actualScore = recordedValue;
}

bool RepairMetric::isTargetAchieved() const noexcept {
    return actualScore >= (targetScore - EPSILON_COMPARISON_TOLERANCE);
}

double RepairMetric::calculateVariancePercentage() const noexcept {
    const double deltaScore = actualScore - targetScore;
    constexpr double PERCENT_FACTOR = 100.0;
    return (deltaScore / targetScore) * PERCENT_FACTOR;
}

// ==========================================
// DiagnosticDevice
// ==========================================

DiagnosticDevice::DiagnosticDevice(int id, std::string serial, std::string model, double initialAccuracy)
    : deviceId(id),
      serialNumber(std::move(serial)),
      deviceModel(std::move(model)),
      accuracyRating(initialAccuracy),
      isCalibratedStatus(true) {
    if (serialNumber.empty() || deviceModel.empty()) {
        throw RepairCompanyException("Device serial number and model cannot be empty.");
    }
    if (initialAccuracy < 0.0 || initialAccuracy > MAXIMUM_ACCURACY_PERCENTAGE) {
        throw RepairCompanyException("Initial device accuracy is outside valid bounds (0-100%).");
    }
}

void DiagnosticDevice::calibrateDevice() {
    accuracyRating = MAXIMUM_ACCURACY_PERCENTAGE;
    isCalibratedStatus = true;
}

bool DiagnosticDevice::isDeviceOperational() const noexcept {
    const bool isAccuracySufficient = (accuracyRating >= MINIMUM_ACCEPTABLE_ACCURACY);
    return isCalibratedStatus && isAccuracySufficient;
}

void DiagnosticDevice::degradeAccuracy(double degradationValue) {
    if (degradationValue < 0.0) {
        throw RepairCompanyException("Accuracy degradation value must be positive.");
    }
    accuracyRating = std::max(0.0, accuracyRating - degradationValue);
    if (accuracyRating < MINIMUM_ACCEPTABLE_ACCURACY) {
        isCalibratedStatus = false;
    }
}

// ==========================================
// DiagnosticsReport
// ==========================================

DiagnosticsReport::DiagnosticsReport(int id, std::string faultDescription, double laborHours,
                                     Technician* specialist, RepairRequest* request)
    : reportId(id),
      detectedFaultDescription(std::move(faultDescription)),
      isDeviceRepairable(true),
      estimatedLaborHours(laborHours),
      diagnosticSpecialist(specialist),
      targetRepairRequest(request) {
    if (detectedFaultDescription.empty()) {
        throw RepairCompanyException("Detected fault description cannot be empty.");
    }
    if (specialist == nullptr || request == nullptr) {
        throw RepairCompanyException("Diagnostics report requires specialist and request references.");
    }
    if (laborHours <= 0.0 || laborHours > MAXIMUM_REASONABLE_LABOR_HOURS) {
        throw RepairCompanyException("Estimated labor hours exceed acceptable standard limits.");
    }
}

void DiagnosticsReport::markAsUnrepairable() {
    isDeviceRepairable = false;
    throw DeviceNotRepairableException("Device has been diagnosed as unrepairable.");
}

void DiagnosticsReport::adjustEstimatedLabor(double revisedLaborHours) {
    if (revisedLaborHours <= 0.0 || revisedLaborHours > MAXIMUM_REASONABLE_LABOR_HOURS) {
        throw RepairCompanyException("Revised labor hours estimate is invalid.");
    }
    estimatedLaborHours = revisedLaborHours;
}

bool DiagnosticsReport::isFeasibleForRepair() const noexcept {
    return isDeviceRepairable;
}

// ==========================================
// QualityInspection
// ==========================================

QualityInspection::QualityInspection(int id, Technician* inspector, WorkOrder* order)
    : inspectionId(id),
      complianceScore(0),
      isQualityApproved(false),
      inspectionNotes(""),
      inspectingTechnician(inspector),
      targetWorkOrder(order) {
    if (inspector == nullptr || order == nullptr) {
        throw QualityCheckFailedException("Quality inspection requires valid technician and work order.");
    }
}

void QualityInspection::evaluateQuality(int score, const std::string& notes) {
    if (score < 0 || score > MAXIMUM_POSSIBLE_SCORE) {
        throw RepairCompanyException("Quality compliance score must be in the range [0, 100].");
    }
    complianceScore = score;
    inspectionNotes = notes;
    if (score < MINIMUM_PASSING_COMPLIANCE_SCORE) {
        isQualityApproved = false;
        throw QualityCheckFailedException("Work order failed the quality compliance standard.");
    }
    isQualityApproved = true;
}

bool QualityInspection::hasPassedInspection() const noexcept {
    return isQualityApproved;
}

void QualityInspection::revokeInspectionApproval() {
    isQualityApproved = false;
}

// ==========================================
// ClientFeedback
// ==========================================

ClientFeedback::ClientFeedback(int id, int rating, std::string comments, WorkOrder* order)
    : feedbackId(id),
      satisfactionScore(rating),
      customerComments(std::move(comments)),
      evaluatedWorkOrder(order) {
    if (order == nullptr) {
        throw OrderNotFoundException("Client feedback requires an associated completed work order.");
    }
    if (rating < MIN_RATING_SCORE || rating > MAX_RATING_SCORE) {
        throw RepairCompanyException("Satisfaction rating must be within the range [1, 5].");
    }
}

bool ClientFeedback::isPositiveReview() const noexcept {
    return satisfactionScore >= POSITIVE_RATING_THRESHOLD;
}

void ClientFeedback::updateReviewComments(const std::string& updatedComments) {
    if (updatedComments.empty()) {
        throw RepairCompanyException("Client review comments cannot be empty.");
    }
    customerComments = updatedComments;
}

// ==========================================
// CommunicationLog
// ==========================================

CommunicationLog::CommunicationLog(int id, std::string channel, std::string content,
                                   bool urgent, Client* client)
    : logId(id),
      contactChannel(std::move(channel)),
      messageContent(std::move(content)),
      isUrgentNotice(urgent),
      associatedClient(client) {
    if (client == nullptr) {
        throw ClientNotFoundException("Communication log must be linked to a registered client.");
    }
    if (messageContent.length() < MINIMUM_VALID_MESSAGE_LENGTH) {
        throw RepairCompanyException("Message content is too short for dispatch.");
    }
}

void CommunicationLog::markAsUrgent() {
    isUrgentNotice = true;
}

bool CommunicationLog::requiresImmediateFollowUp() const noexcept {
    return isUrgentNotice;
}

// ==========================================
// InventoryAudit
// ==========================================

InventoryAudit::InventoryAudit(int id, std::string date, Warehouse* warehouse, Employee* auditor)
    : auditId(id),
      auditDate(std::move(date)),
      foundDiscrepanciesCount(0),
      isAuditApproved(false),
      targetWarehouse(warehouse),
      conductingAuditor(auditor) {
    if (warehouse == nullptr || auditor == nullptr) {
        throw RepairCompanyException("Inventory audit requires warehouse and auditor references.");
    }
    if (auditDate.empty()) {
        throw RepairCompanyException("Audit date cannot be empty.");
    }
}

void InventoryAudit::recordDiscrepancies(int discrepanciesFound) {
    if (discrepanciesFound < 0) {
        throw RepairCompanyException("Discrepancies count cannot be negative.");
    }
    foundDiscrepanciesCount = discrepanciesFound;
}

void InventoryAudit::signAndApproveAudit() {
    if (foundDiscrepanciesCount > ACCEPTABLE_DISCREPANCY_TOLERANCE) {
        throw RepairCompanyException("Cannot approve audit with unresolved discrepancies.");
    }
    isAuditApproved = true;
}

bool InventoryAudit::isAuditCompliant() const noexcept {
    const bool isApproved = isAuditApproved;
    const bool hasNoDiscrepancies = (foundDiscrepanciesCount == ACCEPTABLE_DISCREPANCY_TOLERANCE);
    return isApproved && hasNoDiscrepancies;
}

// ==========================================
// EfficiencyReport
// ==========================================

EfficiencyReport::EfficiencyReport(int id, std::string period, Employee* employee, RepairMetric* metric)
    : reportId(id),
      overallPerformanceRating(0.0),
      evaluationPeriod(std::move(period)),
      evaluatedEmployee(employee),
      trackedMetric(metric) {
    if (employee == nullptr || metric == nullptr) {
        throw RepairCompanyException("Efficiency report requires employee and metric references.");
    }
    if (evaluationPeriod.empty()) {
        throw RepairCompanyException("Evaluation period cannot be empty.");
    }
}

void EfficiencyReport::calculatePerformanceScore(double multiplierFactor) {
    if (multiplierFactor <= 0.0) {
        throw RepairCompanyException("Efficiency multiplier factor must be strictly positive.");
    }
    constexpr double BASE_MAX_SCORE = 100.0;
    const double rawCalculatedScore = BASE_MAX_SCORE * multiplierFactor;
    overallPerformanceRating = std::min(BASE_MAX_SCORE, rawCalculatedScore);
}

bool EfficiencyReport::isEligibleForPerformanceBonus() const noexcept {
    return overallPerformanceRating >= BONUS_THRESHOLD_PERFORMANCE;
}

void EfficiencyReport::linkUpdatedMetric(RepairMetric* newMetric) {
    if (newMetric == nullptr) {
        throw RepairCompanyException("Replacement metric reference cannot be null.");
    }
    trackedMetric = newMetric;
}

// ==========================================
// EquipmentMaintenance
// ==========================================

EquipmentMaintenance::EquipmentMaintenance(int id, std::string date, double cost, Tool* tool, Technician* tech)
    : maintenanceId(id),
      maintenanceDate(std::move(date)),
      maintenanceCost(cost),
      isMaintenanceCompleted(false),
      servicedTool(tool),
      responsibleTechnician(tech) {
    if (tool == nullptr || tech == nullptr) {
        throw ToolNotAvailableException("Equipment maintenance requires tool and technician references.");
    }
    if (cost < 0.0 || cost > MAXIMUM_MAINTENANCE_BUDGET) {
        throw RepairCompanyException("Maintenance cost exceeds allocated budget limit.");
    }
}

void EquipmentMaintenance::completeMaintenanceProcedure() {
    isMaintenanceCompleted = true;
}

void EquipmentMaintenance::updateMaintenanceCost(double additionalCost) {
    if (additionalCost <= 0.0) {
        throw RepairCompanyException("Additional maintenance expenditure must be positive.");
    }
    const double calculatedTotal = maintenanceCost + additionalCost;
    if (calculatedTotal > MAXIMUM_MAINTENANCE_BUDGET) {
        throw RepairCompanyException("Total maintenance cost exceeds allocated budget limit.");
    }
    maintenanceCost = calculatedTotal;
}

bool EquipmentMaintenance::isServiceOverdue() const noexcept {
    return !isMaintenanceCompleted;
}