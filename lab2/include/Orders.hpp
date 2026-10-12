#pragma once

#include <string>
#include <vector>

class Client;
class CorporateClient;
class Technician;
class RepairService;
class SparePart;
class Supplier;
class Invoice;

enum class RequestPriority {
    LOW = 1,
    MEDIUM = 2,
    HIGH = 3,
    URGENT = 4
};

enum class OrderStatus {
    PENDING = 1,
    IN_PROGRESS = 2,
    COMPLETED = 3,
    CANCELLED = 4
};

class RepairRequest {
private:
    static constexpr size_t MIN_DESCRIPTION_LENGTH = 5;

    int requestId;
    std::string problemDescription;
    std::string submissionDate;
    RequestPriority priorityLevel;
    bool isAccepted;
    Client* requestingClient;
    RepairService* requestedService;

public:
    RepairRequest(int id, std::string description, std::string date,
                  RequestPriority priority, Client* client, RepairService* service);

    int getRequestId() const noexcept;
    void acceptRequest();
    void rejectRequest();
    bool isRequestAccepted() const noexcept;
    void escalatePriority(RequestPriority newPriority);

    bool operator==(const RepairRequest& other) const noexcept;
};

class WorkOrder {
private:
    static constexpr size_t MAXIMUM_PARTS_PER_ORDER = 10;

    int orderId;
    std::string creationDate;
    std::string completionDate;
    OrderStatus currentStatus;
    RepairRequest* sourceRequest;
    Technician* assignedTechnician;
    std::vector<SparePart*> consumedParts;
    Invoice* generatedInvoice;

public:
    WorkOrder(int id, std::string date, RepairRequest* request, Technician* technician);

    int getOrderId() const noexcept;
    void startExecution();
    void attachSparePart(SparePart* sparePart);
    void completeExecution(const std::string& finishDate);
    void linkInvoice(Invoice* invoice);
    bool isClosed() const noexcept;

    bool operator==(const WorkOrder& other) const noexcept;
};

class CompletionCertificate {
private:
    int certificateId;
    std::string signingDate;
    bool isSignedByClient;
    WorkOrder* completedOrder;
    Client* signatoryClient;

public:
    CompletionCertificate(int id, std::string date, WorkOrder* order, Client* client);

    int getCertificateId() const noexcept;
    void signCertificate();
    bool isValidForWarranty() const noexcept;

    bool operator==(const CompletionCertificate& other) const noexcept;
};

class WarrantyCard {
private:
    static constexpr int MAXIMUM_WARRANTY_DURATION_DAYS = 365;

    int cardId;
    std::string issueDate;
    int warrantyPeriodDays;
    bool isRevoked;
    WorkOrder* associatedOrder;

public:
    WarrantyCard(int id, std::string date, int durationDays, WorkOrder* order);

    int getCardId() const noexcept;
    void revokeWarranty();
    void extendWarranty(int additionalDays);
    bool isWarrantyActive() const noexcept;

    bool operator==(const WarrantyCard& other) const noexcept;
};

class Contract {
private:
    static constexpr size_t CONTRACT_NUMBER_MIN_LENGTH = 4;

    int contractId;
    std::string contractNumber;
    std::string validUntilDate;
    bool isActive;
    CorporateClient* corporateClient;

public:
    Contract(int id, std::string number, std::string validUntil, CorporateClient* client);

    int getContractId() const noexcept;
    void activateContract();
    void terminateContract();
    bool isContractValid() const noexcept;

    bool operator==(const Contract& other) const noexcept;
};

class ServiceAgreement {
private:
    static constexpr size_t MIN_TERMS_SUMMARY_LENGTH = 5;

    int agreementId;
    std::string amendmentDate;
    std::string termsSummary;
    Contract* parentContract;

public:
    ServiceAgreement(int id, std::string date, std::string terms, Contract* contract);

    int getAgreementId() const noexcept;
    void appendTerms(const std::string& newTerms);
    bool isParentContractActive() const noexcept;

    bool operator==(const ServiceAgreement& other) const noexcept;
};

class PurchaseOrder {
private:
    static constexpr int MINIMUM_PURCHASE_QUANTITY = 1;
    static constexpr double MINIMUM_UNIT_PRICE = 0.5;

    int purchaseOrderId;
    int orderedQuantity;
    double agreedUnitCost;
    bool isDelivered;
    Supplier* targetSupplier;
    SparePart* orderedPart;

public:
    PurchaseOrder(int id, int quantity, double unitCost, Supplier* supplier, SparePart* part);

    int getPurchaseOrderId() const noexcept;
    void confirmDelivery();
    void adjustQuantity(int newQuantity);
    double calculateTotalCost() const noexcept;

    bool operator==(const PurchaseOrder& other) const noexcept;
};