#pragma once

#include <cstddef>
#include <string>
#include <vector>

// Forward declarations of related domain classes
class Technician;

// ==========================================
// SUPPLIER
// ==========================================

class Supplier {
private:
    static constexpr double MINIMUM_ACCEPTABLE_RATING = 2.0;
    static constexpr double MAXIMUM_RATING_SCORE = 5.0;

    int supplierId;
    std::string supplierCompanyName;
    std::string contactPhoneNumber;
    double reliabilityRating;
    bool isCooperationActive;

public:
    Supplier(int id, std::string companyName, std::string phone, double rating);

    int getSupplierId() const noexcept;
    void evaluateReliability(double deliveredOrderRating);
    void updateContactPhone(const std::string& newPhone);
    void suspendCooperation() noexcept;
    bool isPartnerActive() const noexcept;

    bool operator==(const Supplier& other) const noexcept;
};

// ==========================================
// SPARE PART
// ==========================================

class SparePart {
private:
    static constexpr double MINIMUM_PART_PRICE = 0.5;

    int partId;
    std::string partTitle;
    std::string partArticleCode;
    double unitPurchasePrice;
    Supplier* assignedSupplier;

public:
    SparePart(int id, std::string title, std::string articleCode, double price, Supplier* supplier);

    int getPartId() const noexcept;
    double getUnitPrice() const noexcept;
    void adjustPrice(double newPrice);
    void verifySupplierActive() const;
    bool isCompatibleWithModel(const std::string& targetModelCode) const noexcept;

    bool operator==(const SparePart& other) const noexcept;
};

// ==========================================
// BATCH
// ==========================================

class Batch {
private:
    static constexpr int MAXIMUM_SHELF_LIFE_DAYS = 730; // 2 years

    int batchId;
    std::string manufactureDate;
    int initialUnitsQuantity;
    SparePart* suppliedPart;

public:
    Batch(int id, std::string mfgDate, int initialQuantity, SparePart* part);

    int getBatchId() const noexcept;
    bool isBatchExpired(int currentStorageDays) const noexcept;
    double calculateRemainingBatchValue(int remainingUnitsCount) const;

    bool operator==(const Batch& other) const noexcept;
};

// ==========================================
// CONSUMABLE ITEM
// ==========================================

class ConsumableItem {
private:
    static constexpr int MINIMUM_STOCK_ALERT_LIMIT = 5;

    int consumableId;
    std::string itemName;
    int availableUnitsCount;
    std::string measurementUnit;

public:
    ConsumableItem(int id, std::string name, int unitsCount, std::string measurement);

    int getConsumableId() const noexcept;
    void consumeUnits(int consumedAmount);
    void replenishUnits(int deliveredAmount);
    bool isStockDepleted() const noexcept;

    bool operator==(const ConsumableItem& other) const noexcept;
};

// ==========================================
// STORAGE SHELF
// ==========================================

class StorageShelf {
private:
    static constexpr double DEFAULT_MAX_WEIGHT_KG = 150.0;
    static constexpr size_t MAXIMUM_SLOTS_CAPACITY = 30;

    int shelfId;
    std::string locationCode;
    double maximumWeightCapacityKg;
    std::vector<SparePart*> storedSpareParts;

public:
    StorageShelf(int id, std::string code, double maxWeightCapacity);

    int getShelfId() const noexcept;
    void placePart(SparePart* part);
    void removePart(int targetPartId);
    bool isCapacityExceeded() const noexcept;
    size_t getStoredCount() const noexcept;

    bool operator==(const StorageShelf& other) const noexcept;
};

// ==========================================
// WAREHOUSE
// ==========================================

class Warehouse {
private:
    static constexpr size_t MAXIMUM_REGISTERED_SHELVES = 50;

    int warehouseId;
    std::string warehouseAddress;
    std::vector<StorageShelf*> storageShelves;
    std::vector<SparePart*> availableInventory;

public:
    Warehouse(int id, std::string address);

    int getWarehouseId() const noexcept;
    void registerShelf(StorageShelf* shelf);
    void receivePartToStock(SparePart* part, StorageShelf* shelf);
    void issuePartFromStock(int targetPartId);
    bool checkStockAvailability(int targetPartId) const noexcept;

    bool operator==(const Warehouse& other) const noexcept;
};

// ==========================================
// TOOL
// ==========================================

class ToolKeeper;

class Tool {
private:
    static constexpr double MAXIMUM_PERMISSIBLE_WEAR = 80.0;
    static constexpr double WEAR_PERCENTAGE_FULL = 100.0;

    int toolId;
    std::string toolTitle;
    double wearPercentage;
    bool isUnderService;
    ToolKeeper* designatedKeeper;

public:
    Tool(int id, std::string title, double initialWear, ToolKeeper* keeper);

    int getToolId() const noexcept;
    void recordUsageWear(double additionalWearPercentage);
    void sendToMaintenance();
    void markServiceCompleted(double repairedWearLevel);
    bool isFitForOperation() const noexcept;

    bool operator==(const Tool& other) const noexcept;
};

// ==========================================
// TOOL ASSIGNMENT
// ==========================================

class ToolAssignment {
private:
    static constexpr int MAXIMUM_ASSIGNMENT_DAYS = 14;

    int assignmentId;
    std::string assignmentDate;
    Tool* assignedTool;
    Technician* assignedTechnician;

public:
    ToolAssignment(int id, std::string date, Tool* tool, Technician* technician);

    int getAssignmentId() const noexcept;
    void confirmReturn();
    bool isOverdueAssignment(int elapsedDaysCount) const noexcept;
    void extendAssignmentDuration(int addedDaysCount);

    bool operator==(const ToolAssignment& other) const noexcept;
};

// ==========================================
// TOOL KEEPER
// ==========================================

class ToolKeeper {
private:
    static constexpr size_t MAX_MANAGED_TOOLS_LIMIT = 40;

    int keeperId;
    std::string employeeFullName;
    std::vector<Tool*> managedTools;
    StorageShelf* assignedShelf;

public:
    ToolKeeper(int id, std::string name, StorageShelf* shelf);

    int getKeeperId() const noexcept;
    void issueToolToTechnician(Tool* tool, Technician* technician);
    void acceptReturnedTool(Tool* tool);
    void inspectToolCondition(Tool* tool, double measuredWear);

    bool operator==(const ToolKeeper& other) const noexcept;
};