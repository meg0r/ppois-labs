#pragma once

#include <string>

// Forward declarations of related domain classes
class Client;
class Employee;
class Technician;
class RepairRequest;
class WorkOrder;
class Warehouse;
class Tool;

// ==========================================
// WORK SHIFT
// ==========================================

class WorkShift {
private:
    static constexpr int STANDARD_SHIFT_DURATION_HOURS = 8;
    static constexpr int MAXIMUM_PERMISSIBLE_SHIFT_HOURS = 12;

    int shiftId;
    std::string shiftCode;
    int plannedDurationHours;
    bool isShiftActive;

public:
    WorkShift(int id, std::string code, int durationHours);

    int getShiftId() const noexcept;
    void openShift();
    void closeShift();
    bool isActive() const noexcept;
};

// ==========================================
// EFFICIENCY METRICS
// ==========================================

class RepairMetric {
private:
    static constexpr double DEFAULT_TARGET_SCORE = 100.0;
    static constexpr double EPSILON_COMPARISON_TOLERANCE = 0.001;

    int metricId;
    std::string metricName;
    double targetScore;
    double actualScore;

public:
    RepairMetric(int id, std::string name, double target);

    void recordActualScore(double recordedValue);
    bool isTargetAchieved() const noexcept;
    double calculateVariancePercentage() const noexcept;
};

// ==========================================
// DIAGNOSTIC DEVICE
// ==========================================

class DiagnosticDevice {
private:
    static constexpr double MINIMUM_ACCEPTABLE_ACCURACY = 95.0;
    static constexpr double MAXIMUM_ACCURACY_PERCENTAGE = 100.0;

    int deviceId;
    std::string serialNumber;
    std::string deviceModel;
    double accuracyRating;
    bool isCalibratedStatus;

public:
    DiagnosticDevice(int id, std::string serial, std::string model, double initialAccuracy);

    void calibrateDevice();
    bool isDeviceOperational() const noexcept;
    void degradeAccuracy(double degradationValue);
};

// ==========================================
// DIAGNOSTICS REPORT
// ==========================================

class DiagnosticsReport {
private:
    static constexpr double MAXIMUM_REASONABLE_LABOR_HOURS = 50.0;

    int reportId;
    std::string detectedFaultDescription;
    bool isDeviceRepairable;
    double estimatedLaborHours;
    Technician* diagnosticSpecialist;
    RepairRequest* targetRepairRequest;

public:
    DiagnosticsReport(int id, std::string faultDescription, double laborHours,
                      Technician* specialist, RepairRequest* request);

    void markAsUnrepairable();
    void adjustEstimatedLabor(double revisedLaborHours);
    bool isFeasibleForRepair() const noexcept;
};

// ==========================================
// QUALITY CONTROL
// ==========================================

class QualityInspection {
private:
    static constexpr int MINIMUM_PASSING_COMPLIANCE_SCORE = 80;
    static constexpr int MAXIMUM_POSSIBLE_SCORE = 100;

    int inspectionId;
    int complianceScore;
    bool isQualityApproved;
    std::string inspectionNotes;
    Technician* inspectingTechnician;
    WorkOrder* targetWorkOrder;

public:
    QualityInspection(int id, Technician* inspector, WorkOrder* order);

    void evaluateQuality(int score, const std::string& notes);
    bool hasPassedInspection() const noexcept;
    void revokeInspectionApproval();
};

// ==========================================
// CLIENT FEEDBACK
// ==========================================

class ClientFeedback {
private:
    static constexpr int MIN_RATING_SCORE = 1;
    static constexpr int MAX_RATING_SCORE = 5;
    static constexpr int POSITIVE_RATING_THRESHOLD = 4;

    int feedbackId;
    int satisfactionScore;
    std::string customerComments;
    WorkOrder* evaluatedWorkOrder;

public:
    ClientFeedback(int id, int rating, std::string comments, WorkOrder* order);

    bool isPositiveReview() const noexcept;
    void updateReviewComments(const std::string& updatedComments);
};

// ==========================================
// COMMUNICATION LOG
// ==========================================

class CommunicationLog {
private:
    static constexpr size_t MINIMUM_VALID_MESSAGE_LENGTH = 3;

    int logId;
    std::string contactChannel;
    std::string messageContent;
    bool isUrgentNotice;
    Client* associatedClient;

public:
    CommunicationLog(int id, std::string channel, std::string content,
                     bool urgent, Client* client);

    void markAsUrgent();
    bool requiresImmediateFollowUp() const noexcept;
};

// ==========================================
// INVENTORY AUDIT
// ==========================================

class InventoryAudit {
private:
    static constexpr int ACCEPTABLE_DISCREPANCY_TOLERANCE = 0;

    int auditId;
    std::string auditDate;
    int foundDiscrepanciesCount;
    bool isAuditApproved;
    Warehouse* targetWarehouse;
    Employee* conductingAuditor;

public:
    InventoryAudit(int id, std::string date, Warehouse* warehouse, Employee* auditor);

    void recordDiscrepancies(int discrepanciesFound);
    void signAndApproveAudit();
    bool isAuditCompliant() const noexcept;
};

// ==========================================
// EFFICIENCY REPORT
// ==========================================

class EfficiencyReport {
private:
    static constexpr double BONUS_THRESHOLD_PERFORMANCE = 85.0;

    int reportId;
    double overallPerformanceRating;
    std::string evaluationPeriod;
    Employee* evaluatedEmployee;
    RepairMetric* trackedMetric;

public:
    EfficiencyReport(int id, std::string period, Employee* employee, RepairMetric* metric);

    void calculatePerformanceScore(double multiplierFactor);
    bool isEligibleForPerformanceBonus() const noexcept;
    void linkUpdatedMetric(RepairMetric* newMetric);
};

// ==========================================
// EQUIPMENT MAINTENANCE
// ==========================================

class EquipmentMaintenance {
private:
    static constexpr double MAXIMUM_MAINTENANCE_BUDGET = 1500.0;

    int maintenanceId;
    std::string maintenanceDate;
    double maintenanceCost;
    bool isMaintenanceCompleted;
    Tool* servicedTool;
    Technician* responsibleTechnician;

public:
    EquipmentMaintenance(int id, std::string date, double cost, Tool* tool, Technician* tech);

    void completeMaintenanceProcedure();
    void updateMaintenanceCost(double additionalCost);
    bool isServiceOverdue() const noexcept;
};