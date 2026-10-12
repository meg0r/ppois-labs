#include "Actors.hpp"
#include "DiagnosticsQuality.hpp"
#include "Exceptions.hpp"
#include "Finance.hpp"
#include "Orders.hpp"
#include "Services.hpp"
#include "Warehouse.hpp"

#include <algorithm>
#include <utility>

// ==========================================
// Client
// ==========================================

Client::Client(int id, std::string name, std::string phone, std::string regDate)
    : clientId(id),
      fullName(std::move(name)),
      phoneNumber(std::move(phone)),
      registrationDate(std::move(regDate)),
      loyaltyCard(nullptr) {
    if (clientId <= 0) {
        throw RepairCompanyException("Client ID must be positive.");
    }
    if (fullName.empty()) {
        throw RepairCompanyException("Client full name cannot be empty.");
    }
    if (phoneNumber.length() < MIN_PHONE_DIGITS_LENGTH) {
        throw RepairCompanyException("Phone number does not meet minimum length requirement.");
    }
    if (registrationDate.empty()) {
        throw RepairCompanyException("Registration date cannot be empty.");
    }
}

int Client::getClientId() const noexcept {
    return clientId;
}

void Client::attachLoyaltyCard(LoyaltyCard* card) {
    if (card == nullptr) {
        throw RepairCompanyException("Cannot attach a null loyalty card.");
    }
    loyaltyCard = card;
}

bool Client::hasLoyaltyCard() const noexcept {
    return loyaltyCard != nullptr;
}

void Client::updateContactPhone(const std::string& newPhoneNumber) {
    if (newPhoneNumber.length() < MIN_PHONE_DIGITS_LENGTH) {
        throw RepairCompanyException("New phone number is too short.");
    }
    phoneNumber = newPhoneNumber;
}

bool Client::operator==(const Client& other) const noexcept {
    return clientId == other.clientId;
}

// ==========================================
// IndividualClient
// ==========================================

IndividualClient::IndividualClient(int id, std::string name, std::string phone, std::string regDate,
                                   std::string passport, std::string birthDate)
    : Client(id, std::move(name), std::move(phone), std::move(regDate)),
      passportNumber(std::move(passport)),
      dateOfBirth(std::move(birthDate)),
      preferredContactChannel("Phone") {
    if (passportNumber.empty()) {
        throw RepairCompanyException("Passport number cannot be empty.");
    }
    if (dateOfBirth.empty()) {
        throw RepairCompanyException("Date of birth cannot be empty.");
    }
}

bool IndividualClient::verifyIdentity(const std::string& inputPassport) const noexcept {
    return !inputPassport.empty() && passportNumber == inputPassport;
}

void IndividualClient::updateContactChannel(const std::string& channelName) {
    if (channelName.empty()) {
        throw RepairCompanyException("Contact channel name cannot be empty.");
    }
    preferredContactChannel = channelName;
}

// ==========================================
// CorporateClient
// ==========================================

CorporateClient::CorporateClient(int id, std::string name, std::string phone, std::string regDate,
                                 std::string company, std::string taxId, std::string bankDetails)
    : Client(id, std::move(name), std::move(phone), std::move(regDate)),
      companyName(std::move(company)),
      taxIdNumber(std::move(taxId)),
      bankAccountDetails(std::move(bankDetails)),
      activeContract(nullptr) {
    if (companyName.empty() || bankAccountDetails.empty()) {
        throw RepairCompanyException("Company name and bank account details are required.");
    }
    validateTaxIdentifier();
}

void CorporateClient::bindContract(Contract* contract) {
    if (contract == nullptr) {
        throw InvalidContractStatusException("Cannot bind an invalid null contract.");
    }
    activeContract = contract;
}

bool CorporateClient::hasActiveContract() const noexcept {
    return activeContract != nullptr;
}

void CorporateClient::validateTaxIdentifier() const {
    if (taxIdNumber.length() != TAX_ID_REQUIRED_LENGTH) {
        throw RepairCompanyException("Corporate tax ID must contain exactly 9 characters.");
    }
}

// ==========================================
// Employee
// ==========================================

Employee::Employee(int id, std::string name, std::string position, double salaryRate)
    : employeeId(id),
      employeeFullName(std::move(name)),
      jobPosition(std::move(position)),
      baseSalaryRate(salaryRate),
      isActiveEmployee(true),
      currentWorkShift(nullptr) {
    if (employeeId <= 0) {
        throw RepairCompanyException("Employee ID must be positive.");
    }
    if (employeeFullName.empty() || jobPosition.empty()) {
        throw RepairCompanyException("Employee name and position are required.");
    }
    if (salaryRate < MINIMUM_SALARY_RATE) {
        throw RepairCompanyException("Salary rate is below the statutory minimum threshold.");
    }
}

int Employee::getEmployeeId() const noexcept {
    return employeeId;
}

void Employee::assignShift(WorkShift* assignedShift) {
    if (!isActiveEmployee) {
        throw RepairCompanyException("Cannot assign a shift to an inactive employee.");
    }
    if (assignedShift == nullptr) {
        throw RepairCompanyException("Assigned work shift cannot be null.");
    }
    currentWorkShift = assignedShift;
}

void Employee::endCurrentShift() {
    if (currentWorkShift == nullptr) {
        throw RepairCompanyException("Employee has no active shift to end.");
    }
    currentWorkShift = nullptr;
}

void Employee::adjustSalaryRate(double newSalaryRate) {
    if (newSalaryRate < MINIMUM_SALARY_RATE) {
        throw RepairCompanyException("Adjusted salary rate cannot be lower than the minimum threshold.");
    }
    baseSalaryRate = newSalaryRate;
}

void Employee::deactivateEmployment() {
    isActiveEmployee = false;
    currentWorkShift = nullptr;
}

bool Employee::operator==(const Employee& other) const noexcept {
    return employeeId == other.employeeId;
}

// ==========================================
// Technician
// ==========================================

Technician::Technician(int id, std::string name, double salaryRate, int grade)
    : Employee(id, std::move(name), "Technician", salaryRate),
      qualificationGrade(grade),
      isCurrentlyBusy(false),
      completedOrdersCount(0),
      assignedTool(nullptr),
      primarySpecialization(nullptr) {
    if (grade < MIN_QUALIFICATION_GRADE || grade > MAX_QUALIFICATION_GRADE) {
        throw RepairCompanyException("Qualification grade must be within the range [1, 5].");
    }
}

void Technician::assignWorkOrder(WorkOrder* order) {
    if (order == nullptr) {
        throw OrderNotFoundException("Cannot assign a null work order.");
    }
    if (isCurrentlyBusy) {
        throw TechnicianBusyException("Technician is currently occupied with another task.");
    }
    isCurrentlyBusy = true;
}

void Technician::completeCurrentOrder() {
    if (!isCurrentlyBusy) {
        throw RepairCompanyException("Technician has no active work order to finish.");
    }
    isCurrentlyBusy = false;
    ++completedOrdersCount;
}

void Technician::assignTool(Tool* newTool) {
    if (newTool == nullptr) {
        throw ToolNotAvailableException("Cannot assign an unavailable or null tool.");
    }
    assignedTool = newTool;
}

void Technician::releaseTool() {
    assignedTool = nullptr;
}

void Technician::upgradeQualification(int gradeIncrement) {
    if (gradeIncrement <= 0) {
        throw RepairCompanyException("Qualification increment must be strictly positive.");
    }
    const int targetGrade = qualificationGrade + gradeIncrement;
    if (targetGrade > MAX_QUALIFICATION_GRADE) {
        throw RepairCompanyException("Upgraded qualification exceeds maximum attainable grade.");
    }
    qualificationGrade = targetGrade;
}

void Technician::setSpecialization(RepairService* specialization) {
    if (specialization == nullptr) {
        throw RepairCompanyException("Specialization service pointer cannot be null.");
    }
    primarySpecialization = specialization;
}

// ==========================================
// Apprentice
// ==========================================

Apprentice::Apprentice(int id, std::string name, double salaryRate)
    : Technician(id, std::move(name), salaryRate, MIN_QUALIFICATION_GRADE),
      mentorSpecialist(nullptr),
      completedTrainingHours(0),
      targetTrainingHours(DEFAULT_REQUIRED_TRAINING_HOURS),
      isCertifiedTechnician(false) {}

void Apprentice::assignMentor(MasterSpecialist* mentor) {
    if (mentor == nullptr) {
        throw RepairCompanyException("Mentor specialist cannot be null.");
    }
    mentorSpecialist = mentor;
}

void Apprentice::logTrainingHours(int hoursToAdd) {
    if (hoursToAdd <= 0) {
        throw RepairCompanyException("Training hours to record must be strictly positive.");
    }
    completedTrainingHours += hoursToAdd;
    if (completedTrainingHours >= targetTrainingHours) {
        isCertifiedTechnician = true;
    }
}

bool Apprentice::isQualifiedForPromotion() const noexcept {
    return isCertifiedTechnician && (completedTrainingHours >= targetTrainingHours);
}

// ==========================================
// MasterSpecialist
// ==========================================

MasterSpecialist::MasterSpecialist(int id, std::string name, double salaryRate, std::string category)
    : Technician(id, std::move(name), salaryRate, MAX_QUALIFICATION_GRADE),
      expertCategory(std::move(category)),
      inspectionsConductedCount(0),
      lastQualityInspection(nullptr),
      lastReviewedReport(nullptr) {
    if (expertCategory.empty()) {
        throw RepairCompanyException("Expert category specialization cannot be empty.");
    }
}

void MasterSpecialist::conductQualityInspection(QualityInspection* inspection, WorkOrder* order) {
    if (isInspectionLimitReached()) {
        throw RepairCompanyException("Monthly quality inspection capacity has been exceeded.");
    }
    if (inspection == nullptr || order == nullptr) {
        throw QualityCheckFailedException("Inspection and order references must be valid.");
    }
    lastQualityInspection = inspection;
    ++inspectionsConductedCount;
}

void MasterSpecialist::reviewDiagnosticsReport(DiagnosticsReport* report) {
    if (report == nullptr) {
        throw RepairCompanyException("Diagnostics report to review cannot be null.");
    }
    lastReviewedReport = report;
}

bool MasterSpecialist::canMentorApprentices() const noexcept {
    return qualificationGrade >= MIN_GRADE_FOR_MENTORSHIP;
}

bool MasterSpecialist::isInspectionLimitReached() const noexcept {
    return inspectionsConductedCount >= MAXIMUM_MONTHLY_INSPECTIONS;
}

// ==========================================
// Dispatcher
// ==========================================

Dispatcher::Dispatcher(int id, std::string name, double salaryRate, int deskId)
    : Employee(id, std::move(name), "Dispatcher", salaryRate),
      dispatchDeskIdentifier(deskId),
      processedRequestsCount(0),
      isLineAvailable(true),
      operatorExperienceLevel(1) {
    if (deskId <= 0) {
        throw RepairCompanyException("Dispatch desk identifier must be positive.");
    }
}

void Dispatcher::registerRequest(RepairRequest* request, Client* client) {
    verifyClientRegistered(client);
    if (request == nullptr) {
        throw RepairCompanyException("Repair request cannot be null.");
    }
    completeProcessingCycle();
}

void Dispatcher::routeOrderToTechnician(WorkOrder* order, Technician* targetTechnician) {
    if (order == nullptr || targetTechnician == nullptr) {
        throw RepairCompanyException("Work order and target technician cannot be null.");
    }
    targetTechnician->assignWorkOrder(order);
    completeProcessingCycle();
}

void Dispatcher::completeProcessingCycle() {
    ++processedRequestsCount;
    operatorExperienceLevel = 1 + (processedRequestsCount / CALLS_PER_EXPERIENCE_LEVEL);
}

void Dispatcher::verifyClientRegistered(const Client* clientToCheck) const {
    if (clientToCheck == nullptr) {
        throw ClientNotFoundException("Client is not registered in the system.");
    }
}

// ==========================================
// Manager
// ==========================================

Manager::Manager(int id, std::string name, double salaryRate, std::string department, double expenseLimit)
    : Employee(id, std::move(name), "Manager", salaryRate),
      managedDepartment(std::move(department)),
      directSubordinatesCount(0),
      managedLedger(nullptr),
      lastApprovedPurchase(nullptr),
      maximumExpenseLimit(expenseLimit) {
    if (managedDepartment.empty()) {
        throw RepairCompanyException("Managed department designation cannot be empty.");
    }
    if (expenseLimit <= 0.0) {
        throw RepairCompanyException("Expense authorization limit must be strictly positive.");
    }
}

void Manager::approvePurchaseOrder(PurchaseOrder* purchaseOrder, double estimatedTotalCost) {
    authorizeExpense(estimatedTotalCost);
    if (purchaseOrder == nullptr) {
        throw RepairCompanyException("Purchase order pointer cannot be null.");
    }
    lastApprovedPurchase = purchaseOrder;
}

void Manager::registerNewEmployee(Employee* newSubordinate) {
    if (newSubordinate == nullptr) {
        throw RepairCompanyException("New subordinate employee pointer cannot be null.");
    }
    ++directSubordinatesCount;
}

void Manager::linkFinancialLedger(FinancialLedger* ledger) {
    if (ledger == nullptr) {
        throw RepairCompanyException("Financial ledger pointer cannot be null.");
    }
    managedLedger = ledger;
}

void Manager::authorizeExpense(double requestedAmount) const {
    if (requestedAmount <= 0.0) {
        throw RepairCompanyException("Expense amount must be strictly positive.");
    }
    if (requestedAmount > maximumExpenseLimit) {
        throw RepairCompanyException("Requested amount exceeds manager expense limit.");
    }
}

void Manager::dismissSubordinate(Employee* subordinateToDismiss) {
    if (subordinateToDismiss == nullptr) {
        throw RepairCompanyException("Subordinate employee to dismiss cannot be null.");
    }
    if (directSubordinatesCount <= 0) {
        throw RepairCompanyException("No active subordinates assigned to dismiss.");
    }
    subordinateToDismiss->deactivateEmployment();
    --directSubordinatesCount;
}

// ==========================================
// Accountant
// ==========================================

Accountant::Accountant(int id, std::string name, double salaryRate, std::string licenseNumber)
    : Employee(id, std::move(name), "Accountant", salaryRate),
      professionalLicenseNumber(std::move(licenseNumber)),
      auditedInvoicesCount(0),
      lastAuditedTransaction(nullptr),
      lastProcessedSalaryRecord(nullptr) {
    if (professionalLicenseNumber.empty()) {
        throw RepairCompanyException("Professional accounting license cannot be empty.");
    }
}

void Accountant::auditInvoice(Invoice* invoiceToAudit) {
    verifyInvoiceCompliance(invoiceToAudit);
    ++auditedInvoicesCount;
}

void Accountant::processSalaryPayment(Employee* targetEmployee, SalaryRecord* salaryRecord, double amountToPay) {
    if (targetEmployee == nullptr || salaryRecord == nullptr) {
        throw RepairCompanyException("Employee and salary record must be valid.");
    }
    if (amountToPay <= 0.0) {
        throw InsufficientPaymentException("Salary payment amount must be strictly positive.");
    }
    lastProcessedSalaryRecord = salaryRecord;
}

void Accountant::registerPaymentTransaction(PaymentTransaction* transaction) {
    if (transaction == nullptr) {
        throw RepairCompanyException("Payment transaction cannot be null.");
    }
    lastAuditedTransaction = transaction;
}

void Accountant::verifyInvoiceCompliance(const Invoice* targetInvoice) const {
    if (targetInvoice == nullptr) {
        throw OrderNotFoundException("Invoice to audit does not exist.");
    }
}