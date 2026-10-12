#include "Orders.hpp"
#include "Actors.hpp"
#include "Exceptions.hpp"
#include "Finance.hpp"
#include "Services.hpp"
#include "Warehouse.hpp"

#include <utility>

// ==========================================
// RepairRequest
// ==========================================

RepairRequest::RepairRequest(int id, std::string description, std::string date,
                             RequestPriority priority, Client* client, RepairService* service)
    : requestId(id),
      problemDescription(std::move(description)),
      submissionDate(std::move(date)),
      priorityLevel(priority),
      isAccepted(false),
      requestingClient(client),
      requestedService(service) {
    if (client == nullptr) {
        throw ClientNotFoundException("Repair request requires a registered client.");
    }
    if (problemDescription.length() < MIN_DESCRIPTION_LENGTH) {
        throw RepairCompanyException("Problem description is too short.");
    }
}

int RepairRequest::getRequestId() const noexcept {
    return requestId;
}

void RepairRequest::acceptRequest() {
    if (isAccepted) {
        throw RepairCompanyException("This repair request has already been accepted.");
    }
    isAccepted = true;
}

void RepairRequest::rejectRequest() {
    isAccepted = false;
}

bool RepairRequest::isRequestAccepted() const noexcept {
    return isAccepted;
}

void RepairRequest::escalatePriority(RequestPriority newPriority) {
    if (!isAccepted) {
        throw RepairCompanyException("Cannot change priority of an unaccepted request.");
    }
    priorityLevel = newPriority;
}

bool RepairRequest::operator==(const RepairRequest& other) const noexcept {
    return requestId == other.requestId;
}

// ==========================================
// WorkOrder
// ==========================================

WorkOrder::WorkOrder(int id, std::string date, RepairRequest* request, Technician* technician)
    : orderId(id),
      creationDate(std::move(date)),
      completionDate(""),
      currentStatus(OrderStatus::PENDING),
      sourceRequest(request),
      assignedTechnician(technician),
      generatedInvoice(nullptr) {
    if (request == nullptr) {
        throw OrderNotFoundException("Work order cannot be created without a valid request.");
    }
    if (technician == nullptr) {
        throw TechnicianBusyException("Work order requires an assigned technician.");
    }
}

int WorkOrder::getOrderId() const noexcept {
    return orderId;
}

void WorkOrder::startExecution() {
    if (isClosed()) {
        throw OrderAlreadyCompletedException("Cannot start execution on a closed work order.");
    }
    currentStatus = OrderStatus::IN_PROGRESS;
}

void WorkOrder::attachSparePart(SparePart* sparePart) {
    if (sparePart == nullptr) {
        throw SparePartOutOfStockException("Cannot attach a null spare part to work order.");
    }
    if (isClosed()) {
        throw OrderAlreadyCompletedException("Cannot attach parts to a completed work order.");
    }
    if (consumedParts.size() >= MAXIMUM_PARTS_PER_ORDER) {
        throw RepairCompanyException("Maximum parts limit per work order reached.");
    }
    consumedParts.push_back(sparePart);
}

void WorkOrder::completeExecution(const std::string& finishDate) {
    if (isClosed()) {
        throw OrderAlreadyCompletedException("Work order is already completed.");
    }
    if (finishDate.empty()) {
        throw RepairCompanyException("Completion date cannot be empty.");
    }
    completionDate = finishDate;
    currentStatus = OrderStatus::COMPLETED;
}

void WorkOrder::linkInvoice(Invoice* invoice) {
    if (invoice == nullptr) {
        throw OrderNotFoundException("Invoice to link cannot be null.");
    }
    generatedInvoice = invoice;
}

bool WorkOrder::isClosed() const noexcept {
    const bool isCompleted = (currentStatus == OrderStatus::COMPLETED);
    const bool isCancelled = (currentStatus == OrderStatus::CANCELLED);
    return isCompleted || isCancelled;
}

bool WorkOrder::operator==(const WorkOrder& other) const noexcept {
    return orderId == other.orderId;
}

// ==========================================
// CompletionCertificate
// ==========================================

CompletionCertificate::CompletionCertificate(int id, std::string date, WorkOrder* order, Client* client)
    : certificateId(id),
      signingDate(std::move(date)),
      isSignedByClient(false),
      completedOrder(order),
      signatoryClient(client) {
    if (order == nullptr || client == nullptr) {
        throw OrderNotFoundException("Certificate requires valid order and client references.");
    }
    if (signingDate.empty()) {
        throw RepairCompanyException("Certificate signing date cannot be empty.");
    }
}

int CompletionCertificate::getCertificateId() const noexcept {
    return certificateId;
}

void CompletionCertificate::signCertificate() {
    if (!completedOrder->isClosed()) {
        throw RepairCompanyException("Cannot sign certificate before work order completion.");
    }
    isSignedByClient = true;
}

bool CompletionCertificate::isValidForWarranty() const noexcept {
    return isSignedByClient && completedOrder->isClosed();
}

bool CompletionCertificate::operator==(const CompletionCertificate& other) const noexcept {
    return certificateId == other.certificateId;
}

// ==========================================
// WarrantyCard
// ==========================================

WarrantyCard::WarrantyCard(int id, std::string date, int durationDays, WorkOrder* order)
    : cardId(id),
      issueDate(std::move(date)),
      warrantyPeriodDays(durationDays),
      isRevoked(false),
      associatedOrder(order) {
    if (order == nullptr) {
        throw OrderNotFoundException("Warranty card requires an associated work order.");
    }
    if (durationDays <= 0 || durationDays > MAXIMUM_WARRANTY_DURATION_DAYS) {
        throw WarrantyExpiredException("Warranty period duration is invalid.");
    }
}

int WarrantyCard::getCardId() const noexcept {
    return cardId;
}

void WarrantyCard::revokeWarranty() {
    if (isRevoked) {
        throw WarrantyExpiredException("Warranty card is already revoked.");
    }
    isRevoked = true;
}

void WarrantyCard::extendWarranty(int additionalDays) {
    if (isRevoked) {
        throw WarrantyExpiredException("Cannot extend a revoked warranty card.");
    }
    if (additionalDays <= 0) {
        throw RepairCompanyException("Warranty extension days must be positive.");
    }
    const int totalDuration = warrantyPeriodDays + additionalDays;
    if (totalDuration > MAXIMUM_WARRANTY_DURATION_DAYS) {
        throw RepairCompanyException("Extended warranty exceeds maximum allowable duration.");
    }
    warrantyPeriodDays = totalDuration;
}

bool WarrantyCard::isWarrantyActive() const noexcept {
    return !isRevoked && associatedOrder->isClosed();
}

bool WarrantyCard::operator==(const WarrantyCard& other) const noexcept {
    return cardId == other.cardId;
}

// ==========================================
// Contract
// ==========================================

Contract::Contract(int id, std::string number, std::string validUntil, CorporateClient* client)
    : contractId(id),
      contractNumber(std::move(number)),
      validUntilDate(std::move(validUntil)),
      isActive(false),
      corporateClient(client) {
    if (client == nullptr) {
        throw ClientNotFoundException("Corporate contract requires an associated client.");
    }
    if (contractNumber.length() < CONTRACT_NUMBER_MIN_LENGTH) {
        throw InvalidContractStatusException("Contract number is too short.");
    }
}

int Contract::getContractId() const noexcept {
    return contractId;
}

void Contract::activateContract() {
    isActive = true;
}

void Contract::terminateContract() {
    if (!isActive) {
        throw InvalidContractStatusException("Contract is already terminated or inactive.");
    }
    isActive = false;
}

bool Contract::isContractValid() const noexcept {
    return isActive && !validUntilDate.empty();
}

bool Contract::operator==(const Contract& other) const noexcept {
    return contractId == other.contractId;
}

// ==========================================
// ServiceAgreement
// ==========================================

ServiceAgreement::ServiceAgreement(int id, std::string date, std::string terms, Contract* contract)
    : agreementId(id),
      amendmentDate(std::move(date)),
      termsSummary(std::move(terms)),
      parentContract(contract) {
    if (contract == nullptr) {
        throw InvalidContractStatusException("Agreement requires a valid parent contract.");
    }
    if (termsSummary.length() < MIN_TERMS_SUMMARY_LENGTH) {
        throw RepairCompanyException("Agreement terms summary is too short.");
    }
}

int ServiceAgreement::getAgreementId() const noexcept {
    return agreementId;
}

void ServiceAgreement::appendTerms(const std::string& newTerms) {
    if (newTerms.empty()) {
        throw RepairCompanyException("Additional terms cannot be empty.");
    }
    termsSummary += "; " + newTerms;
}

bool ServiceAgreement::isParentContractActive() const noexcept {
    return parentContract->isContractValid();
}

bool ServiceAgreement::operator==(const ServiceAgreement& other) const noexcept {
    return agreementId == other.agreementId;
}

// ==========================================
// PurchaseOrder
// ==========================================

PurchaseOrder::PurchaseOrder(int id, int quantity, double unitCost, Supplier* supplier, SparePart* part)
    : purchaseOrderId(id),
      orderedQuantity(quantity),
      agreedUnitCost(unitCost),
      isDelivered(false),
      targetSupplier(supplier),
      orderedPart(part) {
    if (supplier == nullptr || part == nullptr) {
        throw RepairCompanyException("Purchase order requires a supplier and a spare part.");
    }
    if (quantity < MINIMUM_PURCHASE_QUANTITY) {
        throw RepairCompanyException("Purchase quantity must be at least one unit.");
    }
    if (unitCost < MINIMUM_UNIT_PRICE) {
        throw InsufficientPaymentException("Agreed unit price is below minimum threshold.");
    }
}

int PurchaseOrder::getPurchaseOrderId() const noexcept {
    return purchaseOrderId;
}

void PurchaseOrder::confirmDelivery() {
    if (isDelivered) {
        throw RepairCompanyException("Delivery for this purchase order has already been confirmed.");
    }
    isDelivered = true;
}

void PurchaseOrder::adjustQuantity(int newQuantity) {
    if (isDelivered) {
        throw RepairCompanyException("Cannot modify quantity of an already delivered order.");
    }
    if (newQuantity < MINIMUM_PURCHASE_QUANTITY) {
        throw RepairCompanyException("Quantity must be at least one unit.");
    }
    orderedQuantity = newQuantity;
}

double PurchaseOrder::calculateTotalCost() const noexcept {
    return static_cast<double>(orderedQuantity) * agreedUnitCost;
}

bool PurchaseOrder::operator==(const PurchaseOrder& other) const noexcept {
    return purchaseOrderId == other.purchaseOrderId;
}