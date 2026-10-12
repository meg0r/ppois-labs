#include "Warehouse.hpp"
#include "Actors.hpp"
#include "Exceptions.hpp"

#include <algorithm>
#include <utility>

// ==========================================
// Supplier
// ==========================================

Supplier::Supplier(int id, std::string companyName, std::string phone, double rating)
    : supplierId(id),
      supplierCompanyName(std::move(companyName)),
      contactPhoneNumber(std::move(phone)),
      reliabilityRating(rating),
      isCooperationActive(true) {
    if (supplierId <= 0) {
        throw RepairCompanyException("Supplier ID must be positive.");
    }
    if (supplierCompanyName.empty()) {
        throw RepairCompanyException("Supplier company name cannot be empty.");
    }
    if (rating < MINIMUM_ACCEPTABLE_RATING || rating > MAXIMUM_RATING_SCORE) {
        throw RepairCompanyException("Supplier reliability rating is out of allowed range.");
    }
}

int Supplier::getSupplierId() const noexcept {
    return supplierId;
}

void Supplier::evaluateReliability(double deliveredOrderRating) {
    if (deliveredOrderRating < 0.0 || deliveredOrderRating > MAXIMUM_RATING_SCORE) {
        throw RepairCompanyException("Evaluation score must be between 0 and 5.");
    }
    constexpr double WEIGHT_CURRENT = 0.7;
    constexpr double WEIGHT_NEW = 0.3;
    reliabilityRating = (reliabilityRating * WEIGHT_CURRENT) + (deliveredOrderRating * WEIGHT_NEW);
}

void Supplier::updateContactPhone(const std::string& newPhone) {
    if (newPhone.empty()) {
        throw RepairCompanyException("Contact phone cannot be empty.");
    }
    contactPhoneNumber = newPhone;
}

void Supplier::suspendCooperation() noexcept {
    isCooperationActive = false;
}

bool Supplier::isPartnerActive() const noexcept {
    return isCooperationActive;
}

bool Supplier::operator==(const Supplier& other) const noexcept {
    return supplierId == other.supplierId;
}

// ==========================================
// SparePart
// ==========================================

SparePart::SparePart(int id, std::string title, std::string articleCode, double price, Supplier* supplier)
    : partId(id),
      partTitle(std::move(title)),
      partArticleCode(std::move(articleCode)),
      unitPurchasePrice(price),
      assignedSupplier(supplier) {
    if (partId <= 0) {
        throw RepairCompanyException("Spare part ID must be positive.");
    }
    if (partTitle.empty() || partArticleCode.empty()) {
        throw RepairCompanyException("Part title and article code are mandatory.");
    }
    if (price < MINIMUM_PART_PRICE) {
        throw RepairCompanyException("Spare part price is below minimal threshold.");
    }
    if (supplier == nullptr) {
        throw RepairCompanyException("Spare part must be bound to a supplier.");
    }
}

int SparePart::getPartId() const noexcept {
    return partId;
}

double SparePart::getUnitPrice() const noexcept {
    return unitPurchasePrice;
}

void SparePart::adjustPrice(double newPrice) {
    if (newPrice < MINIMUM_PART_PRICE) {
        throw RepairCompanyException("Updated part price is below minimal threshold.");
    }
    unitPurchasePrice = newPrice;
}

void SparePart::verifySupplierActive() const {
    if (!assignedSupplier->isPartnerActive()) {
        throw RepairCompanyException("Associated supplier cooperation is suspended.");
    }
}

bool SparePart::isCompatibleWithModel(const std::string& targetModelCode) const noexcept {
    return !targetModelCode.empty() && partArticleCode.find(targetModelCode) != std::string::npos;
}

bool SparePart::operator==(const SparePart& other) const noexcept {
    return partId == other.partId;
}

// ==========================================
// Batch
// ==========================================

Batch::Batch(int id, std::string mfgDate, int initialQuantity, SparePart* part)
    : batchId(id),
      manufactureDate(std::move(mfgDate)),
      initialUnitsQuantity(initialQuantity),
      suppliedPart(part) {
    if (batchId <= 0) {
        throw RepairCompanyException("Batch ID must be positive.");
    }
    if (manufactureDate.empty()) {
        throw RepairCompanyException("Batch manufacture date cannot be empty.");
    }
    if (initialQuantity <= 0) {
        throw RepairCompanyException("Initial batch quantity must be positive.");
    }
    if (part == nullptr) {
        throw SparePartOutOfStockException("Batch requires a valid spare part reference.");
    }
}

int Batch::getBatchId() const noexcept {
    return batchId;
}

bool Batch::isBatchExpired(int currentStorageDays) const noexcept {
    return currentStorageDays > MAXIMUM_SHELF_LIFE_DAYS;
}

double Batch::calculateRemainingBatchValue(int remainingUnitsCount) const {
    if (remainingUnitsCount < 0 || remainingUnitsCount > initialUnitsQuantity) {
        throw RepairCompanyException("Remaining units count is outside valid bounds.");
    }
    return static_cast<double>(remainingUnitsCount) * suppliedPart->getUnitPrice();
}

bool Batch::operator==(const Batch& other) const noexcept {
    return batchId == other.batchId;
}

// ==========================================
// ConsumableItem
// ==========================================

ConsumableItem::ConsumableItem(int id, std::string name, int unitsCount, std::string measurement)
    : consumableId(id),
      itemName(std::move(name)),
      availableUnitsCount(unitsCount),
      measurementUnit(std::move(measurement)) {
    if (consumableId <= 0) {
        throw RepairCompanyException("Consumable item ID must be positive.");
    }
    if (itemName.empty() || measurementUnit.empty()) {
        throw RepairCompanyException("Item name and measurement unit are required.");
    }
    if (unitsCount < 0) {
        throw RepairCompanyException("Consumable initial units cannot be negative.");
    }
}

int ConsumableItem::getConsumableId() const noexcept {
    return consumableId;
}

void ConsumableItem::consumeUnits(int consumedAmount) {
    if (consumedAmount <= 0) {
        throw RepairCompanyException("Consumed quantity must be strictly positive.");
    }
    if (consumedAmount > availableUnitsCount) {
        throw SparePartOutOfStockException("Insufficient consumable units in stock.");
    }
    availableUnitsCount -= consumedAmount;
}

void ConsumableItem::replenishUnits(int deliveredAmount) {
    if (deliveredAmount <= 0) {
        throw RepairCompanyException("Replenished quantity must be positive.");
    }
    availableUnitsCount += deliveredAmount;
}

bool ConsumableItem::isStockDepleted() const noexcept {
    return availableUnitsCount <= MINIMUM_STOCK_ALERT_LIMIT;
}

bool ConsumableItem::operator==(const ConsumableItem& other) const noexcept {
    return consumableId == other.consumableId;
}

// ==========================================
// StorageShelf
// ==========================================

StorageShelf::StorageShelf(int id, std::string code, double maxWeightCapacity)
    : shelfId(id),
      locationCode(std::move(code)),
      maximumWeightCapacityKg(maxWeightCapacity) {
    if (shelfId <= 0) {
        throw RepairCompanyException("Storage shelf ID must be positive.");
    }
    if (locationCode.empty()) {
        throw RepairCompanyException("Shelf location code cannot be empty.");
    }
    if (maxWeightCapacity <= 0.0) {
        throw RepairCompanyException("Maximum weight capacity must be positive.");
    }
}

int StorageShelf::getShelfId() const noexcept {
    return shelfId;
}

void StorageShelf::placePart(SparePart* part) {
    if (part == nullptr) {
        throw SparePartOutOfStockException("Cannot place null part on shelf.");
    }
    if (isCapacityExceeded()) {
        throw RepairCompanyException("Shelf capacity limit reached.");
    }
    storedSpareParts.push_back(part);
}

void StorageShelf::removePart(int targetPartId) {
    auto match = std::find_if(storedSpareParts.begin(), storedSpareParts.end(),
        [targetPartId](const SparePart* p) { return p->getPartId() == targetPartId; });
    if (match == storedSpareParts.end()) {
        throw SparePartOutOfStockException("Target part not found on shelf.");
    }
    storedSpareParts.erase(match);
}

bool StorageShelf::isCapacityExceeded() const noexcept {
    return storedSpareParts.size() >= MAXIMUM_SLOTS_CAPACITY;
}

size_t StorageShelf::getStoredCount() const noexcept {
    return storedSpareParts.size();
}

bool StorageShelf::operator==(const StorageShelf& other) const noexcept {
    return shelfId == other.shelfId;
}

// ==========================================
// Warehouse
// ==========================================

Warehouse::Warehouse(int id, std::string address)
    : warehouseId(id),
      warehouseAddress(std::move(address)) {
    if (warehouseId <= 0) {
        throw RepairCompanyException("Warehouse ID must be positive.");
    }
    if (warehouseAddress.empty()) {
        throw RepairCompanyException("Warehouse address cannot be empty.");
    }
}

int Warehouse::getWarehouseId() const noexcept {
    return warehouseId;
}

void Warehouse::registerShelf(StorageShelf* shelf) {
    if (shelf == nullptr) {
        throw RepairCompanyException("Shelf pointer to register cannot be null.");
    }
    if (storageShelves.size() >= MAXIMUM_REGISTERED_SHELVES) {
        throw RepairCompanyException("Maximum shelf limit for warehouse exceeded.");
    }
    storageShelves.push_back(shelf);
}

void Warehouse::receivePartToStock(SparePart* part, StorageShelf* shelf) {
    if (part == nullptr || shelf == nullptr) {
        throw SparePartOutOfStockException("Valid part and shelf references are required.");
    }
    shelf->placePart(part);
    availableInventory.push_back(part);
}

void Warehouse::issuePartFromStock(int targetPartId) {
    auto match = std::find_if(availableInventory.begin(), availableInventory.end(),
        [targetPartId](const SparePart* p) { return p->getPartId() == targetPartId; });
    if (match == availableInventory.end()) {
        throw SparePartOutOfStockException("Requested part is not available in warehouse.");
    }
    for (auto* shelf : storageShelves) {
        try {
            shelf->removePart(targetPartId);
            break;
        } catch (const SparePartOutOfStockException&) {
            continue;
        }
    }
    availableInventory.erase(match);
}

bool Warehouse::checkStockAvailability(int targetPartId) const noexcept {
    return std::any_of(availableInventory.begin(), availableInventory.end(),
        [targetPartId](const SparePart* p) { return p->getPartId() == targetPartId; });
}

bool Warehouse::operator==(const Warehouse& other) const noexcept {
    return warehouseId == other.warehouseId;
}

// ==========================================
// Tool
// ==========================================

Tool::Tool(int id, std::string title, double initialWear, ToolKeeper* keeper)
    : toolId(id),
      toolTitle(std::move(title)),
      wearPercentage(initialWear),
      isUnderService(false),
      designatedKeeper(keeper) {
    if (toolId <= 0) {
        throw RepairCompanyException("Tool ID must be positive.");
    }
    if (toolTitle.empty()) {
        throw RepairCompanyException("Tool title cannot be empty.");
    }
    if (initialWear < 0.0 || initialWear > WEAR_PERCENTAGE_FULL) {
        throw RepairCompanyException("Tool wear must be in range 0-100%.");
    }
}

int Tool::getToolId() const noexcept {
    return toolId;
}

void Tool::recordUsageWear(double additionalWearPercentage) {
    if (additionalWearPercentage < 0.0) {
        throw RepairCompanyException("Wear percentage addition cannot be negative.");
    }
    wearPercentage = std::min(WEAR_PERCENTAGE_FULL, wearPercentage + additionalWearPercentage);
    if (wearPercentage > MAXIMUM_PERMISSIBLE_WEAR) {
        sendToMaintenance();
    }
}

void Tool::sendToMaintenance() {
    isUnderService = true;
}

void Tool::markServiceCompleted(double repairedWearLevel) {
    if (repairedWearLevel < 0.0 || repairedWearLevel > MAXIMUM_PERMISSIBLE_WEAR) {
        throw RepairCompanyException("Repaired wear level must be within safe operational limits.");
    }
    wearPercentage = repairedWearLevel;
    isUnderService = false;
}

bool Tool::isFitForOperation() const noexcept {
    return !isUnderService && (wearPercentage <= MAXIMUM_PERMISSIBLE_WEAR);
}

bool Tool::operator==(const Tool& other) const noexcept {
    return toolId == other.toolId;
}

// ==========================================
// ToolAssignment
// ==========================================

ToolAssignment::ToolAssignment(int id, std::string date, Tool* tool, Technician* technician)
    : assignmentId(id),
      assignmentDate(std::move(date)),
      assignedTool(tool),
      assignedTechnician(technician) {
    if (assignmentId <= 0) {
        throw RepairCompanyException("Assignment ID must be positive.");
    }
    if (tool == nullptr || technician == nullptr) {
        throw ToolNotAvailableException("Valid tool and technician are required.");
    }
    if (!tool->isFitForOperation()) {
        throw ToolNotAvailableException("Tool is not fit for operational deployment.");
    }
}

int ToolAssignment::getAssignmentId() const noexcept {
    return assignmentId;
}

void ToolAssignment::confirmReturn() {
    if (assignedTool == nullptr) {
        throw ToolNotAvailableException("Tool assignment has already been returned.");
    }
    assignedTool = nullptr;
}

bool ToolAssignment::isOverdueAssignment(int elapsedDaysCount) const noexcept {
    return elapsedDaysCount > MAXIMUM_ASSIGNMENT_DAYS;
}

void ToolAssignment::extendAssignmentDuration(int addedDaysCount) {
    if (addedDaysCount <= 0) {
        throw RepairCompanyException("Assignment extension days must be positive.");
    }
    if (addedDaysCount > MAXIMUM_ASSIGNMENT_DAYS) {
        throw RepairCompanyException("Cannot extend assignment past the maximum allowed duration.");
    }
}

bool ToolAssignment::operator==(const ToolAssignment& other) const noexcept {
    return assignmentId == other.assignmentId;
}

// ==========================================
// ToolKeeper
// ==========================================

ToolKeeper::ToolKeeper(int id, std::string name, StorageShelf* shelf)
    : keeperId(id),
      employeeFullName(std::move(name)),
      assignedShelf(shelf) {
    if (keeperId <= 0) {
        throw RepairCompanyException("Tool keeper ID must be positive.");
    }
    if (employeeFullName.empty()) {
        throw RepairCompanyException("Tool keeper name cannot be empty.");
    }
    if (shelf == nullptr) {
        throw RepairCompanyException("Assigned storage shelf cannot be null.");
    }
}

int ToolKeeper::getKeeperId() const noexcept {
    return keeperId;
}

void ToolKeeper::issueToolToTechnician(Tool* tool, Technician* technician) {
    if (tool == nullptr || technician == nullptr) {
        throw ToolNotAvailableException("Valid tool and technician pointers required.");
    }
    if (!tool->isFitForOperation()) {
        throw ToolNotAvailableException("Tool is currently under service or excessive wear.");
    }
    technician->assignTool(tool);
}

void ToolKeeper::acceptReturnedTool(Tool* tool) {
    if (tool == nullptr) {
        throw ToolNotAvailableException("Returned tool cannot be null.");
    }
    if (managedTools.size() >= MAX_MANAGED_TOOLS_LIMIT) {
        throw RepairCompanyException("Managed tool capacity limit reached.");
    }
    managedTools.push_back(tool);
}

void ToolKeeper::inspectToolCondition(Tool* tool, double measuredWear) {
    if (tool == nullptr) {
        throw ToolNotAvailableException("Target tool for inspection cannot be null.");
    }
    tool->recordUsageWear(measuredWear);
}

bool ToolKeeper::operator==(const ToolKeeper& other) const noexcept {
    return keeperId == other.keeperId;
}