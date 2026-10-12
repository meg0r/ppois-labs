#pragma once

#include <memory>
#include <string>
#include <vector>

class Client;
class Employee;
class Technician;
class Dispatcher;
class Accountant;
class Manager;
class Warehouse;
class RepairRequest;
class WorkOrder;
class Invoice;
class FinancialLedger;

class CLI {
public:
    enum class MenuOption : int {
        EXIT = 0,
        REGISTER_CLIENT = 1,
        ADD_EMPLOYEE = 2,
        CREATE_REPAIR_REQUEST = 3,
        ASSIGN_WORK_ORDER = 4,
        CONSUME_SPARE_PART = 5,
        INSPECT_AND_FINALIZE_ORDER = 6,
        PROCESS_PAYMENT = 7,
        CALCULATE_SALARIES = 8,
        SHOW_LEDGER_REPORT = 9
    };

private:
    static constexpr int MENU_MIN_OPTION = 0;
    static constexpr int MENU_MAX_OPTION = 9;

    bool isRunning;

    std::vector<std::unique_ptr<Client>> registeredClients;
    std::vector<std::unique_ptr<Employee>> staffMembers;
    std::vector<std::unique_ptr<RepairRequest>> activeRequests;
    std::vector<std::unique_ptr<WorkOrder>> activeOrders;
    std::vector<std::unique_ptr<Invoice>> issuedInvoices;

    std::unique_ptr<Warehouse> companyWarehouse;
    std::unique_ptr<FinancialLedger> companyLedger;

    void printHeader() const;
    void printMenu() const;

    void processUserChoice(int choiceCode);
    void handleRegisterClient();
    void handleAddEmployee();
    void handleCreateRepairRequest();
    void handleAssignWorkOrder();
    void handleConsumeSparePart();
    void handleInspectAndFinalizeOrder();
    void handleProcessPayment();
    void handleCalculateSalaries();
    void handleShowLedgerReport() const;

    int readInteger(const std::string& prompt) const;
    double readDouble(const std::string& prompt) const;
    std::string readString(const std::string& prompt) const;

    Client* findClientById(int identifier) const;
    Technician* findTechnicianById(int identifier) const;
    WorkOrder* findOrderById(int identifier) const;

    void seedInitialDemoData();

public:
    CLI();
    ~CLI();

    CLI(const CLI&) = delete;
    CLI& operator=(const CLI&) = delete;

    CLI(CLI&&) noexcept = default;
    CLI& operator=(CLI&&) noexcept = default;

    void run();
};