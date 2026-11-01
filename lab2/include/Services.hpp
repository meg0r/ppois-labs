#pragma once

#include <string>
#include <vector>

// Forward declarations of related domain classes
class DiagnosticDevice;

// ==========================================
// SERVICE CATEGORY
// ==========================================

class ServiceCategory {
private:
    static constexpr size_t MIN_CATEGORY_NAME_LENGTH = 3;

    int categoryId;
    std::string categoryName;
    bool isActiveCategory;

public:
    ServiceCategory(int id, std::string name);

    int getCategoryId() const noexcept;
    void renameCategory(const std::string& newName);
    void deactivateCategory() noexcept;
    bool isActive() const noexcept;

    bool operator==(const ServiceCategory& other) const noexcept;
};

// ==========================================
// REPAIR SERVICE
// ==========================================

class RepairService {
protected:
    static constexpr double MINIMUM_BASE_PRICE = 5.0;
    static constexpr double MINIMUM_DURATION_HOURS = 0.5;
    static constexpr double MAXIMUM_DURATION_HOURS = 100.0;

    int serviceId;
    std::string serviceName;
    double basePrice;
    double estimatedDurationHours;
    ServiceCategory* serviceCategory;

public:
    RepairService(int id, std::string name, double price, double durationHours, ServiceCategory* category);
    virtual ~RepairService() = default;

    int getServiceId() const noexcept;
    double getBasePrice() const noexcept;
    void updateBasePrice(double newPrice);
    void assignCategory(ServiceCategory* newCategory);
    double calculateServiceCost(double hourlyLaborMultiplier) const;

    bool operator==(const RepairService& other) const noexcept;
};

// ==========================================
// URGENT REPAIR SERVICE
// ==========================================

class UrgentRepairService : public RepairService {
private:
    static constexpr double DEFAULT_URGENCY_SURCHARGE_RATE = 1.5;
    static constexpr double MIN_SURCHARGE_RATE = 1.1;
    static constexpr double MAX_SURCHARGE_RATE = 3.0;

    double urgencySurchargeRate;
    double guaranteedCompletionHours;

public:
    UrgentRepairService(int id, std::string name, double price, double durationHours,
                        ServiceCategory* category, double surchargeRate, double guaranteedHours);

    double calculateUrgentCost() const noexcept;
    bool isExpeditedDeliveryGuaranteed() const noexcept;
};

// ==========================================
// DIAGNOSTIC SERVICE
// ==========================================

class DiagnosticService {
private:
    static constexpr size_t MIN_PROTOCOL_NAME_LENGTH = 3;

    int diagnosticServiceId;
    std::string testProtocolName;
    bool hardwareTestRequired;
    DiagnosticDevice* assignedDevice;

public:
    DiagnosticService(int id, std::string protocolName, bool requiresHardwareTest);

    int getDiagnosticServiceId() const noexcept;
    void bindDiagnosticDevice(DiagnosticDevice* device);
    void validateEquipmentSuitability() const;
    bool isHardwareTestMandatory() const noexcept;

    bool operator==(const DiagnosticService& other) const noexcept;
};

// ==========================================
// PRICE LIST
// ==========================================

class PriceList {
private:
    static constexpr double MIN_INDEXATION_PERCENT = -20.0;
    static constexpr double MAX_INDEXATION_PERCENT = 50.0;
    static constexpr double PERCENT_DIVISOR = 100.0;

    int priceListId;
    std::string effectiveDate;
    std::vector<RepairService*> registeredServices;

public:
    PriceList(int id, std::string date);

    int getPriceListId() const noexcept;
    void registerService(RepairService* service);
    RepairService* findServiceById(int targetServiceId) const;
    void applyPriceIndexation(double percentageRate);
    size_t getServicesCount() const noexcept;

    bool operator==(const PriceList& other) const noexcept;
};