#pragma once

#include <stdexcept>
#include <string>

// Base class for service center domain exceptions
class RepairCompanyException : public std::runtime_error {
public:
    explicit RepairCompanyException(const std::string& message)
        : std::runtime_error(message) {}
    explicit RepairCompanyException(const char* message)
        : std::runtime_error(message) {}
};

// 1. Device is unrepairable
class DeviceNotRepairableException : public RepairCompanyException {
public:
    explicit DeviceNotRepairableException(
        const char* message = "Device is deemed unrepairable based on diagnostic results."
    ) : RepairCompanyException(message) {}
    explicit DeviceNotRepairableException(const std::string& message)
        : RepairCompanyException(message) {}
};

// 2. Spare part is out of stock
class SparePartOutOfStockException : public RepairCompanyException {
public:
    explicit SparePartOutOfStockException(
        const char* message = "Requested spare part is out of stock in the warehouse."
    ) : RepairCompanyException(message) {}
    explicit SparePartOutOfStockException(const std::string& message)
        : RepairCompanyException(message) {}
};

// 3. Technician is busy
class TechnicianBusyException : public RepairCompanyException {
public:
    explicit TechnicianBusyException(
        const char* message = "Selected technician is busy with another work order."
    ) : RepairCompanyException(message) {}
    explicit TechnicianBusyException(const std::string& message)
        : RepairCompanyException(message) {}
};

// 4. Warranty expired
class WarrantyExpiredException : public RepairCompanyException {
public:
    explicit WarrantyExpiredException(
        const char* message = "Warranty period for completed work has expired."
    ) : RepairCompanyException(message) {}
    explicit WarrantyExpiredException(const std::string& message)
        : RepairCompanyException(message) {}
};

// 5. Order already completed
class OrderAlreadyCompletedException : public RepairCompanyException {
public:
    explicit OrderAlreadyCompletedException(
        const char* message = "Order is already completed and cannot be modified."
    ) : RepairCompanyException(message) {}
    explicit OrderAlreadyCompletedException(const std::string& message)
        : RepairCompanyException(message) {}
};

// 6. Order not found
class OrderNotFoundException : public RepairCompanyException {
public:
    explicit OrderNotFoundException(
        const char* message = "Order with the specified identifier was not found."
    ) : RepairCompanyException(message) {}
    explicit OrderNotFoundException(const std::string& message)
        : RepairCompanyException(message) {}
};

// 7. Client not found
class ClientNotFoundException : public RepairCompanyException {
public:
    explicit ClientNotFoundException(
        const char* message = "Client with the specified details is not registered."
    ) : RepairCompanyException(message) {}
    explicit ClientNotFoundException(const std::string& message)
        : RepairCompanyException(message) {}
};

// 8. Insufficient payment
class InsufficientPaymentException : public RepairCompanyException {
public:
    explicit InsufficientPaymentException(
        const char* message = "Tendered payment amount is insufficient to settle the invoice."
    ) : RepairCompanyException(message) {}
    explicit InsufficientPaymentException(const std::string& message)
        : RepairCompanyException(message) {}
};

// 9. Contract status error
class InvalidContractStatusException : public RepairCompanyException {
public:
    explicit InvalidContractStatusException(
        const char* message = "Client contract is inactive or terminated."
    ) : RepairCompanyException(message) {}
    explicit InvalidContractStatusException(const std::string& message)
        : RepairCompanyException(message) {}
};

// 10. Tool unavailable
class ToolNotAvailableException : public RepairCompanyException {
public:
    explicit ToolNotAvailableException(
        const char* message = "Tool is under maintenance or assigned to another technician."
    ) : RepairCompanyException(message) {}
    explicit ToolNotAvailableException(const std::string& message)
        : RepairCompanyException(message) {}
};

// 11. Invalid discount
class InvalidDiscountException : public RepairCompanyException {
public:
    explicit InvalidDiscountException(
        const char* message = "Discount rate is invalid or exceeds the allowed limit."
    ) : RepairCompanyException(message) {}
    explicit InvalidDiscountException(const std::string& message)
        : RepairCompanyException(message) {}
};

// 12. Quality check failed
class QualityCheckFailedException : public RepairCompanyException {
public:
    explicit QualityCheckFailedException(
        const char* message = "Repair failed quality control and was returned for rework."
    ) : RepairCompanyException(message) {}
    explicit QualityCheckFailedException(const std::string& message)
        : RepairCompanyException(message) {}
};