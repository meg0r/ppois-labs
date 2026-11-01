#include "Services.hpp"
#include "DiagnosticsQuality.hpp"
#include "Exceptions.hpp"

#include <algorithm>
#include <utility>

// ==========================================
// ServiceCategory
// ==========================================

ServiceCategory::ServiceCategory(int id, std::string name)
    : categoryId(id),
      categoryName(std::move(name)),
      isActiveCategory(true) {
    if (categoryId <= 0) {
        throw RepairCompanyException("Service category ID must be positive.");
    }
    if (categoryName.length() < MIN_CATEGORY_NAME_LENGTH) {
        throw RepairCompanyException("Category name does not meet minimum length requirement.");
    }
}

int ServiceCategory::getCategoryId() const noexcept {
    return categoryId;
}

void ServiceCategory::renameCategory(const std::string& newName) {
    if (newName.length() < MIN_CATEGORY_NAME_LENGTH) {
        throw RepairCompanyException("New category name is too short.");
    }
    categoryName = newName;
}

void ServiceCategory::deactivateCategory() noexcept {
    isActiveCategory = false;
}

bool ServiceCategory::isActive() const noexcept {
    return isActiveCategory;
}

bool ServiceCategory::operator==(const ServiceCategory& other) const noexcept {
    return categoryId == other.categoryId;
}

// ==========================================
// RepairService
// ==========================================

RepairService::RepairService(int id, std::string name, double price, double durationHours, ServiceCategory* category)
    : serviceId(id),
      serviceName(std::move(name)),
      basePrice(price),
      estimatedDurationHours(durationHours),
      serviceCategory(category) {
    if (serviceId <= 0) {
        throw RepairCompanyException("Service ID must be positive.");
    }
    if (serviceName.empty()) {
        throw RepairCompanyException("Service designation cannot be empty.");
    }
    if (price < MINIMUM_BASE_PRICE) {
        throw RepairCompanyException("Base price is below statutory minimum rate.");
    }
    if (durationHours < MINIMUM_DURATION_HOURS || durationHours > MAXIMUM_DURATION_HOURS) {
        throw RepairCompanyException("Estimated labor duration is outside allowed boundaries.");
    }
}

int RepairService::getServiceId() const noexcept {
    return serviceId;
}

double RepairService::getBasePrice() const noexcept {
    return basePrice;
}

void RepairService::updateBasePrice(double newPrice) {
    if (newPrice < MINIMUM_BASE_PRICE) {
        throw RepairCompanyException("Updated price is below statutory minimum rate.");
    }
    basePrice = newPrice;
}

void RepairService::assignCategory(ServiceCategory* newCategory) {
    if (newCategory == nullptr) {
        throw RepairCompanyException("Service category pointer cannot be null.");
    }
    serviceCategory = newCategory;
}

double RepairService::calculateServiceCost(double hourlyLaborMultiplier) const {
    if (hourlyLaborMultiplier <= 0.0) {
        throw RepairCompanyException("Labor hourly multiplier must be strictly positive.");
    }
    return basePrice * hourlyLaborMultiplier;
}

bool RepairService::operator==(const RepairService& other) const noexcept {
    return serviceId == other.serviceId;
}

// ==========================================
// UrgentRepairService
// ==========================================

UrgentRepairService::UrgentRepairService(int id, std::string name, double price, double durationHours,
                                         ServiceCategory* category, double surchargeRate, double guaranteedHours)
    : RepairService(id, std::move(name), price, durationHours, category),
      urgencySurchargeRate(surchargeRate),
      guaranteedCompletionHours(guaranteedHours) {
    if (surchargeRate < MIN_SURCHARGE_RATE || surchargeRate > MAX_SURCHARGE_RATE) {
        throw InvalidDiscountException("Urgency surcharge rate is outside permissible limits.");
    }
    if (guaranteedHours <= 0.0) {
        throw RepairCompanyException("Guaranteed completion hours must be strictly positive.");
    }
}

double UrgentRepairService::calculateUrgentCost() const noexcept {
    return basePrice * urgencySurchargeRate;
}

bool UrgentRepairService::isExpeditedDeliveryGuaranteed() const noexcept {
    return guaranteedCompletionHours <= estimatedDurationHours;
}

// ==========================================
// DiagnosticService
// ==========================================

DiagnosticService::DiagnosticService(int id, std::string protocolName, bool requiresHardwareTest)
    : diagnosticServiceId(id),
      testProtocolName(std::move(protocolName)),
      hardwareTestRequired(requiresHardwareTest),
      assignedDevice(nullptr) {
    if (diagnosticServiceId <= 0) {
        throw RepairCompanyException("Diagnostic service ID must be positive.");
    }
    if (testProtocolName.length() < MIN_PROTOCOL_NAME_LENGTH) {
        throw RepairCompanyException("Diagnostic protocol name is too short.");
    }
}

int DiagnosticService::getDiagnosticServiceId() const noexcept {
    return diagnosticServiceId;
}

void DiagnosticService::bindDiagnosticDevice(DiagnosticDevice* device) {
    if (device == nullptr) {
        throw ToolNotAvailableException("Diagnostic device pointer cannot be null.");
    }
    assignedDevice = device;
}

void DiagnosticService::validateEquipmentSuitability() const {
    if (hardwareTestRequired && assignedDevice == nullptr) {
        throw ToolNotAvailableException("Hardware diagnostic test requires an assigned device.");
    }
    if (assignedDevice != nullptr && !assignedDevice->isDeviceOperational()) {
        throw ToolNotAvailableException("Assigned diagnostic device is uncalibrated or inoperable.");
    }
}

bool DiagnosticService::isHardwareTestMandatory() const noexcept {
    return hardwareTestRequired;
}

bool DiagnosticService::operator==(const DiagnosticService& other) const noexcept {
    return diagnosticServiceId == other.diagnosticServiceId;
}

// ==========================================
// PriceList
// ==========================================

PriceList::PriceList(int id, std::string date)
    : priceListId(id),
      effectiveDate(std::move(date)) {
    if (priceListId <= 0) {
        throw RepairCompanyException("Price list ID must be positive.");
    }
    if (effectiveDate.empty()) {
        throw RepairCompanyException("Price list effective date cannot be empty.");
    }
}

int PriceList::getPriceListId() const noexcept {
    return priceListId;
}

void PriceList::registerService(RepairService* service) {
    if (service == nullptr) {
        throw RepairCompanyException("Service to register cannot be null.");
    }
    registeredServices.push_back(service);
}

RepairService* PriceList::findServiceById(int targetServiceId) const {
    for (auto* service : registeredServices) {
        if (service->getServiceId() == targetServiceId) {
            return service;
        }
    }
    return nullptr;
}

void PriceList::applyPriceIndexation(double percentageRate) {
    if (percentageRate < MIN_INDEXATION_PERCENT || percentageRate > MAX_INDEXATION_PERCENT) {
        throw RepairCompanyException("Price indexation rate is outside allowable adjustment range.");
    }
    const double indexationMultiplier = 1.0 + (percentageRate / PERCENT_DIVISOR);
    for (auto* currentService : registeredServices) {
        const double calculatedNewPrice = currentService->getBasePrice() * indexationMultiplier;
        currentService->updateBasePrice(calculatedNewPrice);
    }
}

size_t PriceList::getServicesCount() const noexcept {
    return registeredServices.size();
}

bool PriceList::operator==(const PriceList& other) const noexcept {
    return priceListId == other.priceListId;
}