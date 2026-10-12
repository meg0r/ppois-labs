#pragma once

#include <string>

// Forward declarations of related domain classes
class LoyaltyCard;
class Contract;
class WorkShift;
class Tool;
class WorkOrder;
class RepairService;
class RepairRequest;
class PurchaseOrder;
class FinancialLedger;
class Invoice;
class PaymentTransaction;
class SalaryRecord;
class QualityInspection;
class DiagnosticsReport;

// ==========================================
// CLIENTS HIERARCHY
// ==========================================

class Client {
protected:
    static constexpr size_t MIN_PHONE_DIGITS_LENGTH = 7;

    int clientId;
    std::string fullName;
    std::string phoneNumber;
    std::string registrationDate;
    LoyaltyCard* loyaltyCard;

public:
    Client(int id, std::string name, std::string phone, std::string regDate);
    virtual ~Client() = default;

    int getClientId() const noexcept;
    void attachLoyaltyCard(LoyaltyCard* card);
    bool hasLoyaltyCard() const noexcept;
    void updateContactPhone(const std::string& newPhoneNumber);

    bool operator==(const Client& other) const noexcept;
};

class IndividualClient : public Client {
private:
    std::string passportNumber;
    std::string dateOfBirth;
    std::string preferredContactChannel;

public:
    IndividualClient(int id, std::string name, std::string phone, std::string regDate,
                     std::string passport, std::string birthDate);

    bool verifyIdentity(const std::string& inputPassport) const noexcept;
    void updateContactChannel(const std::string& channelName);
};

class CorporateClient : public Client {
private:
    static constexpr size_t TAX_ID_REQUIRED_LENGTH = 9;

    std::string companyName;
    std::string taxIdNumber;
    std::string bankAccountDetails;
    Contract* activeContract;

public:
    CorporateClient(int id, std::string name, std::string phone, std::string regDate,
                    std::string company, std::string taxId, std::string bankDetails);

    void bindContract(Contract* contract);
    bool hasActiveContract() const noexcept;
    void validateTaxIdentifier() const;
};

// ==========================================
// EMPLOYEES HIERARCHY
// ==========================================

class Employee {
protected:
    static constexpr double MINIMUM_SALARY_RATE = 500.0;

    int employeeId;
    std::string employeeFullName;
    std::string jobPosition;
    double baseSalaryRate;
    bool isActiveEmployee;
    WorkShift* currentWorkShift;

public:
    Employee(int id, std::string name, std::string position, double salaryRate);
    virtual ~Employee() = default;

    int getEmployeeId() const noexcept;
    void assignShift(WorkShift* assignedShift);
    void endCurrentShift();
    void adjustSalaryRate(double newSalaryRate);
    void deactivateEmployment();

    bool operator==(const Employee& other) const noexcept;
};

class MasterSpecialist;

class Technician : public Employee {
protected:
    static constexpr int MIN_QUALIFICATION_GRADE = 1;
    static constexpr int MAX_QUALIFICATION_GRADE = 5;

    int qualificationGrade;
    bool isCurrentlyBusy;
    int completedOrdersCount;
    Tool* assignedTool;
    RepairService* primarySpecialization;

public:
    Technician(int id, std::string name, double salaryRate, int grade);

    void assignWorkOrder(WorkOrder* order);
    void completeCurrentOrder();
    void assignTool(Tool* newTool);
    void releaseTool();
    void upgradeQualification(int gradeIncrement);
    void setSpecialization(RepairService* specialization);
};

class Apprentice : public Technician {
private:
    static constexpr int DEFAULT_REQUIRED_TRAINING_HOURS = 160;

    MasterSpecialist* mentorSpecialist;
    int completedTrainingHours;
    int targetTrainingHours;
    bool isCertifiedTechnician;

public:
    Apprentice(int id, std::string name, double salaryRate);

    void assignMentor(MasterSpecialist* mentor);
    void logTrainingHours(int hoursToAdd);
    bool isQualifiedForPromotion() const noexcept;
};

class MasterSpecialist : public Technician {
private:
    static constexpr int MIN_GRADE_FOR_MENTORSHIP = 4;
    static constexpr int MAXIMUM_MONTHLY_INSPECTIONS = 100;

    std::string expertCategory;
    int inspectionsConductedCount;
    QualityInspection* lastQualityInspection;
    DiagnosticsReport* lastReviewedReport;

public:
    MasterSpecialist(int id, std::string name, double salaryRate, std::string category);

    void conductQualityInspection(QualityInspection* inspection, WorkOrder* order);
    void reviewDiagnosticsReport(DiagnosticsReport* report);
    bool canMentorApprentices() const noexcept;
    bool isInspectionLimitReached() const noexcept;
};

class Dispatcher : public Employee {
private:
    static constexpr int CALLS_PER_EXPERIENCE_LEVEL = 50;

    int dispatchDeskIdentifier;
    int processedRequestsCount;
    bool isLineAvailable;
    int operatorExperienceLevel;

public:
    Dispatcher(int id, std::string name, double salaryRate, int deskId);

    void registerRequest(RepairRequest* request, Client* client);
    void routeOrderToTechnician(WorkOrder* order, Technician* targetTechnician);
    void completeProcessingCycle();
    void verifyClientRegistered(const Client* clientToCheck) const;
};

class Manager : public Employee {
private:
    std::string managedDepartment;
    int directSubordinatesCount;
    FinancialLedger* managedLedger;
    PurchaseOrder* lastApprovedPurchase;
    double maximumExpenseLimit;

public:
    Manager(int id, std::string name, double salaryRate, std::string department, double expenseLimit);

    void approvePurchaseOrder(PurchaseOrder* purchaseOrder, double estimatedTotalCost);
    void registerNewEmployee(Employee* newSubordinate);
    void linkFinancialLedger(FinancialLedger* ledger);
    void authorizeExpense(double requestedAmount) const;
    void dismissSubordinate(Employee* subordinateToDismiss);
};

class Accountant : public Employee {
private:
    std::string professionalLicenseNumber;
    int auditedInvoicesCount;
    PaymentTransaction* lastAuditedTransaction;
    SalaryRecord* lastProcessedSalaryRecord;

public:
    Accountant(int id, std::string name, double salaryRate, std::string licenseNumber);

    void auditInvoice(Invoice* invoiceToAudit);
    void processSalaryPayment(Employee* targetEmployee, SalaryRecord* salaryRecord, double amountToPay);
    void registerPaymentTransaction(PaymentTransaction* transaction);
    void verifyInvoiceCompliance(const Invoice* targetInvoice) const;
};