#include "CLI.hpp"
#include "Actors.hpp"
#include "DiagnosticsQuality.hpp"
#include "Exceptions.hpp"
#include "Finance.hpp"
#include "Orders.hpp"
#include "Services.hpp"
#include "Warehouse.hpp"

#include <iostream>
#include <limits>
#include <utility>

CLI::CLI() : isRunning(false) {
    companyWarehouse = std::make_unique<Warehouse>(1, "Main Service Hub, Depot 1");
    companyLedger = std::make_unique<FinancialLedger>(1, "2026-Q4");
    seedInitialDemoData();
}

CLI::~CLI() = default;

void CLI::seedInitialDemoData() {
    auto tech = std::make_unique<Technician>(101, "John Smith", 850.0, 4);
    staffMembers.push_back(std::move(tech));

    auto client = std::make_unique<IndividualClient>(
        201, "Alice Cooper", "+1555019234", "2026-10-01", "ID-992384", "1994-05-12"
    );
    registeredClients.push_back(std::move(client));

    auto shelf = new StorageShelf(1, "SEC-A-01", 150.0);
    companyWarehouse->registerShelf(shelf);
}

void CLI::run() {
    isRunning = true;
    while (isRunning) {
        printHeader();
        printMenu();
        const int choice = readInteger("Select an option");
        try {
            processUserChoice(choice);
        } catch (const RepairCompanyException& domainError) {
            std::cout << "\n[Error] " << domainError.what() << "\n\n";
        }
    }
}

void CLI::printHeader() const {
    std::cout << "========================================\n"
              << "   Service Center Management System     \n"
              << "========================================\n";
}

void CLI::printMenu() const {
    std::cout << " 1. Register New Client\n"
              << " 2. Add New Technician\n"
              << " 3. Create Repair Request\n"
              << " 4. Assign Work Order to Technician\n"
              << " 5. Attach Spare Part to Order\n"
              << " 6. Inspect & Finalize Work Order\n"
              << " 7. Process Invoice Payment\n"
              << " 8. Calculate Staff Salaries\n"
              << " 9. Display Financial Ledger Report\n"
              << " 0. Exit Application\n"
              << "----------------------------------------\n";
}

void CLI::processUserChoice(int choiceCode) {
    if (choiceCode < MENU_MIN_OPTION || choiceCode > MENU_MAX_OPTION) {
        std::cout << "Invalid selection. Please choose an option from 0 to 9.\n";
        return;
    }
    const auto option = static_cast<MenuOption>(choiceCode);
    if (option == MenuOption::EXIT) { isRunning = false; return; }
    if (option == MenuOption::REGISTER_CLIENT) { handleRegisterClient(); return; }
    if (option == MenuOption::ADD_EMPLOYEE) { handleAddEmployee(); return; }
    if (option == MenuOption::CREATE_REPAIR_REQUEST) { handleCreateRepairRequest(); return; }
    if (option == MenuOption::ASSIGN_WORK_ORDER) { handleAssignWorkOrder(); return; }
    if (option == MenuOption::CONSUME_SPARE_PART) { handleConsumeSparePart(); return; }
    if (option == MenuOption::INSPECT_AND_FINALIZE_ORDER) { handleInspectAndFinalizeOrder(); return; }
    if (option == MenuOption::PROCESS_PAYMENT) { handleProcessPayment(); return; }
    if (option == MenuOption::CALCULATE_SALARIES) { handleCalculateSalaries(); return; }
    if (option == MenuOption::SHOW_LEDGER_REPORT) { handleShowLedgerReport(); return; }
}

void CLI::handleRegisterClient() {
    const int clientId = readInteger("Enter client ID");
    const std::string name = readString("Enter client full name");
    const std::string phone = readString("Enter phone number");
    const std::string regDate = readString("Enter registration date (YYYY-MM-DD)");
    const std::string passport = readString("Enter passport / national ID number");
    const std::string birthDate = readString("Enter date of birth (YYYY-MM-DD)");

    auto client = std::make_unique<IndividualClient>(clientId, name, phone, regDate, passport, birthDate);
    registeredClients.push_back(std::move(client));
    std::cout << "[Success] Client registered successfully.\n\n";
}

void CLI::handleAddEmployee() {
    const int employeeId = readInteger("Enter technician employee ID");
    const std::string name = readString("Enter technician full name");
    const double salaryRate = readDouble("Enter monthly base salary rate");
    const int grade = readInteger("Enter qualification grade (1 to 5)");

    auto tech = std::make_unique<Technician>(employeeId, name, salaryRate, grade);
    staffMembers.push_back(std::move(tech));
    std::cout << "[Success] Technician added to active roster.\n\n";
}

void CLI::handleCreateRepairRequest() {
    const int clientId = readInteger("Enter client ID");
    Client* client = findClientById(clientId);
    if (client == nullptr) {
        throw ClientNotFoundException("Client not found with the specified ID.");
    }

    const int requestId = readInteger("Enter request ID");
    const std::string description = readString("Enter problem description");
    const std::string date = readString("Enter submission date (YYYY-MM-DD)");

    auto request = std::make_unique<RepairRequest>(
        requestId, description, date, RequestPriority::HIGH, client, nullptr
    );
    request->acceptRequest();
    activeRequests.push_back(std::move(request));
    std::cout << "[Success] Repair request accepted into backlog.\n\n";
}

void CLI::handleAssignWorkOrder() {
    const int requestId = readInteger("Enter repair request ID");
    RepairRequest* matchedRequest = nullptr;
    for (const auto& req : activeRequests) {
        if (req->getRequestId() == requestId) { matchedRequest = req.get(); break; }
    }
    if (matchedRequest == nullptr) {
        throw OrderNotFoundException("Active repair request not found.");
    }

    const int techId = readInteger("Enter technician ID");
    Technician* technician = findTechnicianById(techId);
    if (technician == nullptr) {
        throw TechnicianBusyException("Assigned technician is unavailable or does not exist.");
    }

    const int orderId = readInteger("Enter new work order ID");
    const std::string orderDate = readString("Enter order start date (YYYY-MM-DD)");
    auto order = std::make_unique<WorkOrder>(orderId, orderDate, matchedRequest, technician);
    order->startExecution();
    activeOrders.push_back(std::move(order));
    std::cout << "[Success] Work order assigned and execution started.\n\n";
}

void CLI::handleConsumeSparePart() {
    const int orderId = readInteger("Enter work order ID");
    WorkOrder* targetOrder = findOrderById(orderId);
    if (targetOrder == nullptr) {
        throw OrderNotFoundException("Work order not found.");
    }

    const int partId = readInteger("Enter spare part ID");
    const double unitPrice = readDouble("Enter spare part unit price");
    auto supplier = new Supplier(1, "OriginalParts LLC", "+1555123456", 4.9);
    auto part = new SparePart(partId, "Replacement Module", "MOD-09", unitPrice, supplier);

    targetOrder->attachSparePart(part);
    std::cout << "[Success] Spare part attached to the work order.\n\n";
}

void CLI::handleInspectAndFinalizeOrder() {
    const int orderId = readInteger("Enter work order ID to finalize");
    WorkOrder* targetOrder = findOrderById(orderId);
    if (targetOrder == nullptr) {
        throw OrderNotFoundException("Target work order does not exist.");
    }

    const int techId = readInteger("Enter inspector technician ID");
    Technician* inspector = findTechnicianById(techId);
    if (inspector == nullptr) {
        throw TechnicianBusyException("Inspector technician not found.");
    }

    auto inspection = std::make_unique<QualityInspection>(1, inspector, targetOrder);
    const int score = readInteger("Enter quality compliance score (0-100)");
    inspection->evaluateQuality(score, "Standard quality audit procedure");

    const std::string completionDate = readString("Enter completion date (YYYY-MM-DD)");
    targetOrder->completeExecution(completionDate);

    constexpr double BASE_LABOR_COST = 140.0;
    constexpr double PARTS_COST = 60.0;
    Client* client = registeredClients.empty() ? nullptr : registeredClients.front().get();
    auto invoice = std::make_unique<Invoice>(orderId, BASE_LABOR_COST, PARTS_COST, client, targetOrder);
    targetOrder->linkInvoice(invoice.get());

    std::cout << "[Success] Order completed. Invoice #" << invoice->getInvoiceId()
              << " generated for $" << invoice->calculateFinalPayableAmount() << "\n\n";
    issuedInvoices.push_back(std::move(invoice));
}

void CLI::handleProcessPayment() {
    const int invoiceId = readInteger("Enter invoice ID to settle");
    Invoice* matchedInvoice = nullptr;
    for (const auto& inv : issuedInvoices) {
        if (inv->getInvoiceId() == invoiceId) { matchedInvoice = inv.get(); break; }
    }
    if (matchedInvoice == nullptr) {
        throw OrderNotFoundException("Target invoice not found.");
    }

    const double amount = matchedInvoice->calculateFinalPayableAmount();
    const std::string date = readString("Enter payment date (YYYY-MM-DD)");
    auto payment = std::make_unique<CashPayment>(1, date, amount, matchedInvoice, amount, "RCP-10492");
    payment->processTransaction();

    companyLedger->recordIncome(amount);
    std::cout << "[Success] Payment settled. Ledger income balance updated.\n\n";
}

void CLI::handleCalculateSalaries() {
    if (staffMembers.empty()) {
        std::cout << "No employees registered for payroll calculation.\n\n";
        return;
    }
    double totalPayrollExpense = 0.0;
    for (const auto& employee : staffMembers) {
        constexpr double STANDARD_BONUS = 50.0;
        constexpr double STANDARD_BASE = 800.0;
        SalaryRecord record(employee->getEmployeeId(), STANDARD_BASE, employee.get());
        record.addBonus(STANDARD_BONUS);
        const double netSalary = record.calculateNetSalary();
        totalPayrollExpense += netSalary;
    }
    companyLedger->recordExpense(totalPayrollExpense);
    std::cout << "[Success] Payroll calculated and recorded. Total payout: $"
              << totalPayrollExpense << "\n\n";
}

void CLI::handleShowLedgerReport() const {
    const double netProfit = companyLedger->calculateNetProfit();
    const bool isReconciled = companyLedger->reconcileLedger();

    std::cout << "\n--- Financial Ledger Performance Report ---\n"
              << " Net Operating Profit: $" << netProfit << '\n'
              << " Reconciliation Status: " << (isReconciled ? "PASSED" : "FAILED") << '\n'
              << "--------------------------------------------\n\n";
}

Client* CLI::findClientById(int identifier) const {
    for (const auto& client : registeredClients) {
        if (client->getClientId() == identifier) { return client.get(); }
    }
    return nullptr;
}

Technician* CLI::findTechnicianById(int identifier) const {
    for (const auto& employee : staffMembers) {
        if (employee->getEmployeeId() == identifier) {
            return dynamic_cast<Technician*>(employee.get());
        }
    }
    return nullptr;
}

WorkOrder* CLI::findOrderById(int identifier) const {
    for (const auto& order : activeOrders) {
        if (order->getOrderId() == identifier) { return order.get(); }
    }
    return nullptr;
}

int CLI::readInteger(const std::string& prompt) const {
    int value = 0;
    while (true) {
        std::cout << prompt << ": ";
        if (std::cin >> value) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid integer. Please try again.\n";
    }
}

double CLI::readDouble(const std::string& prompt) const {
    double value = 0.0;
    while (true) {
        std::cout << prompt << ": ";
        if (std::cin >> value) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid number. Please try again.\n";
    }
}

std::string CLI::readString(const std::string& prompt) const {
    std::string value;
    std::cout << prompt << ": ";
    std::getline(std::cin, value);
    return value;
}