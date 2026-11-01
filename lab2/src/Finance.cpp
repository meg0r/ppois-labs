#include "Finance.hpp"
#include "Exceptions.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <utility>

// ==========================================
// LoyaltyCard
// ==========================================

LoyaltyCard::LoyaltyCard(int id)
    : cardId(id),
      currentDiscountPercent(INITIAL_DISCOUNT_PERCENT),
      totalSpentAccumulated(0.0),
      isActiveCard(true) {
    if (id <= 0) {
        throw RepairCompanyException("Loyalty card ID must be positive.");
    }
}

int LoyaltyCard::getCardId() const noexcept {
    return cardId;
}

void LoyaltyCard::accumulateSpending(double paymentAmount) {
    if (!isActiveCard) {
        throw InvalidDiscountException("Cannot accumulate spending on an inactive card.");
    }
    if (paymentAmount <= 0.0) {
        throw InsufficientPaymentException("Transaction amount must be positive.");
    }
    totalSpentAccumulated += paymentAmount;
}

bool LoyaltyCard::isEligibleForTierUpgrade() const noexcept {
    const bool hasEnoughSpending = (totalSpentAccumulated >= TIER_UPGRADE_SPENDING_THRESHOLD);
    const bool hasDiscountRoom = (currentDiscountPercent < MAXIMUM_TIER_DISCOUNT_PERCENT);
    return isActiveCard && hasEnoughSpending && hasDiscountRoom;
}

void LoyaltyCard::upgradeDiscountTier(double discountIncrement) {
    if (!isActiveCard) {
        throw InvalidDiscountException("Loyalty card is locked against upgrades.");
    }
    if (discountIncrement <= 0.0) {
        throw InvalidDiscountException("Discount increment must be a positive number.");
    }
    const double calculatedDiscount = currentDiscountPercent + discountIncrement;
    if (calculatedDiscount > MAXIMUM_TIER_DISCOUNT_PERCENT) {
        throw InvalidDiscountException("New discount tier exceeds the maximum program limit.");
    }
    currentDiscountPercent = calculatedDiscount;
}

// ==========================================
// DiscountPolicy
// ==========================================

DiscountPolicy::DiscountPolicy(int id, std::string name, double maxCap)
    : policyId(id),
      policyName(std::move(name)),
      maximumDiscountCap(maxCap) {
    if (policyName.empty()) {
        throw RepairCompanyException("Discount policy name is required.");
    }
    if (maxCap < MINIMUM_APPLICABLE_DISCOUNT || maxCap > MAXIMUM_ALLOWED_DISCOUNT_CAP) {
        throw InvalidDiscountException("Discount cap exceeds allowable limits (0-25%).");
    }
}

void DiscountPolicy::validateDiscountRate(double requestedDiscountRate) const {
    if (requestedDiscountRate < MINIMUM_APPLICABLE_DISCOUNT) {
        throw InvalidDiscountException("Discount rate cannot be negative.");
    }
    if (requestedDiscountRate > maximumDiscountCap) {
        throw InvalidDiscountException("Requested discount exceeds the maximum limit for this policy.");
    }
}

double DiscountPolicy::calculateAdjustedDiscount(double requestedRate, const LoyaltyCard* clientCard) const {
    validateDiscountRate(requestedRate);
    if (clientCard == nullptr) {
        return requestedRate;
    }
    constexpr double LOYALTY_COMBINED_MULTIPLIER = 1.1;
    const double adjustedRate = requestedRate * LOYALTY_COMBINED_MULTIPLIER;
    return std::min(maximumDiscountCap, adjustedRate);
}

bool DiscountPolicy::isPolicyApplicable(double orderTotalAmount) const noexcept {
    return orderTotalAmount > 0.0;
}

// ==========================================
// Invoice
// ==========================================

Invoice::Invoice(int id, double laborCost, double partsCost, Client* client, WorkOrder* order)
    : invoiceId(id),
      baseLaborAmount(laborCost),
      partsCostAmount(partsCost),
      appliedDiscountPercentage(0.0),
      calculatedTaxAmount(0.0),
      isInvoiceSettled(false),
      billedClient(client),
      targetWorkOrder(order) {
    if (client == nullptr || order == nullptr) {
        throw OrderNotFoundException("Invoice must be linked to a valid client and work order.");
    }
    if (laborCost < 0.0 || partsCost < 0.0) {
        throw RepairCompanyException("Invoice cost components cannot be negative.");
    }
    const double initialSubtotal = baseLaborAmount + partsCostAmount;
    calculatedTaxAmount = initialSubtotal * STANDARD_TAX_RATE;
}

int Invoice::getInvoiceId() const noexcept {
    return invoiceId;
}

void Invoice::applyDiscountRate(double discountRate) {
    if (isInvoiceSettled) {
        throw OrderAlreadyCompletedException("Invoice is already settled and locked against modifications.");
    }
    if (discountRate < 0.0 || discountRate > 50.0) {
        throw InvalidDiscountException("Discount rate is outside the acceptable range (0-50%).");
    }
    appliedDiscountPercentage = discountRate;
    constexpr double PERCENT_DIVISOR = 100.0;
    const double discountFactor = 1.0 - (discountRate / PERCENT_DIVISOR);
    const double discountedSubtotal = (baseLaborAmount + partsCostAmount) * discountFactor;
    calculatedTaxAmount = discountedSubtotal * STANDARD_TAX_RATE;
}

double Invoice::calculateFinalPayableAmount() const noexcept {
    constexpr double PERCENT_DIVISOR = 100.0;
    const double grossAmount = baseLaborAmount + partsCostAmount;
    const double discountAmount = grossAmount * (appliedDiscountPercentage / PERCENT_DIVISOR);
    const double netPayableSubtotal = grossAmount - discountAmount;
    return netPayableSubtotal + calculatedTaxAmount;
}

void Invoice::markAsSettled() {
    if (isInvoiceSettled) {
        throw OrderAlreadyCompletedException("This invoice has already been marked as settled.");
    }
    isInvoiceSettled = true;
}

bool Invoice::isSettled() const noexcept {
    return isInvoiceSettled;
}

bool Invoice::operator==(const Invoice& other) const noexcept {
    return invoiceId == other.invoiceId;
}

// ==========================================
// PaymentTransaction
// ==========================================

PaymentTransaction::PaymentTransaction(int id, std::string date, double amount, Invoice* invoice)
    : transactionId(id),
      transactionDate(std::move(date)),
      transferredAmount(amount),
      isSuccessful(false),
      associatedInvoice(invoice) {
    if (invoice == nullptr) {
        throw OrderNotFoundException("Payment transaction must be linked to an invoice.");
    }
    if (amount < MINIMUM_TRANSACTION_AMOUNT) {
        throw InsufficientPaymentException("Transaction amount is below the minimum allowed threshold.");
    }
}

int PaymentTransaction::getTransactionId() const noexcept {
    return transactionId;
}

void PaymentTransaction::processTransaction() {
    if (isSuccessful) {
        throw RepairCompanyException("This payment transaction has already been processed.");
    }
    verifyPaymentIntegrity();
    associatedInvoice->markAsSettled();
    isSuccessful = true;
}

void PaymentTransaction::refundTransaction() {
    if (!isSuccessful) {
        throw RepairCompanyException("Cannot issue a refund for an unprocessed transaction.");
    }
    isSuccessful = false;
}

void PaymentTransaction::verifyPaymentIntegrity() const {
    if (transactionDate.empty()) {
        throw RepairCompanyException("Transaction date is required for auditing purposes.");
    }
    if (transferredAmount < MINIMUM_TRANSACTION_AMOUNT) {
        throw InsufficientPaymentException("Payment amount integrity validation failed.");
    }
}

bool PaymentTransaction::operator==(const PaymentTransaction& other) const noexcept {
    return transactionId == other.transactionId;
}

// ==========================================
// CashPayment
// ==========================================

CashPayment::CashPayment(int id, std::string date, double amount, Invoice* invoice,
                         double tendered, std::string receiptNum)
    : PaymentTransaction(id, std::move(date), amount, invoice),
      cashTendered(tendered),
      changeDueAmount(0.0),
      fiscalReceiptNumber(std::move(receiptNum)) {
    if (fiscalReceiptNumber.empty()) {
        throw RepairCompanyException("Fiscal receipt number is required for cash payments.");
    }
}

double CashPayment::calculateChangeAmount() {
    if (cashTendered < transferredAmount) {
        throw InsufficientPaymentException("Tendered cash is insufficient to cover the payment amount.");
    }
    changeDueAmount = cashTendered - transferredAmount;
    return changeDueAmount;
}

void CashPayment::printFiscalReceiptDetails() const {
    if (fiscalReceiptNumber.empty()) {
        throw RepairCompanyException("Fiscal receipt is not registered in the cash register.");
    }
    if (!isSuccessful) {
        throw RepairCompanyException("Cannot print fiscal receipt for an unprocessed payment.");
    }
}

// ==========================================
// CardPayment
// ==========================================

CardPayment::CardPayment(int id, std::string date, double amount, Invoice* invoice,
                         std::string authCode, std::string terminalId,
                         std::string maskedCard, bool contactless)
    : PaymentTransaction(id, std::move(date), amount, invoice),
      authorizationCode(std::move(authCode)),
      terminalIdentifier(std::move(terminalId)),
      maskedCardNumber(std::move(maskedCard)),
      isContactless(contactless) {
    if (terminalIdentifier.empty() || maskedCardNumber.empty()) {
        throw RepairCompanyException("POS terminal details and masked card number are required.");
    }
    validateAuthorizationCode();
}

void CardPayment::authorizeTerminalPayment() {
    validateAuthorizationCode();
    processTransaction();
}

void CardPayment::validateAuthorizationCode() const {
    if (authorizationCode.length() != REQUIRED_AUTH_CODE_LENGTH) {
        throw InsufficientPaymentException("Terminal authorization code must be exactly 6 characters long.");
    }
    for (const char authChar : authorizationCode) {
        const auto unsignedChar = static_cast<unsigned char>(authChar);
        if (!std::isalnum(unsignedChar)) {
            throw InsufficientPaymentException("Authorization code contains invalid special characters.");
        }
    }
}

// ==========================================
// ExpenseItem
// ==========================================

ExpenseItem::ExpenseItem(int id, std::string category, double amount, std::string date)
    : expenseId(id),
      categoryName(std::move(category)),
      expenseAmount(amount),
      expenseDate(std::move(date)) {
    if (categoryName.empty() || expenseDate.empty()) {
        throw RepairCompanyException("Expense category and date are required.");
    }
    if (amount <= 0.0) {
        throw RepairCompanyException("Expense amount must be strictly positive.");
    }
}

int ExpenseItem::getExpenseId() const noexcept {
    return expenseId;
}

void ExpenseItem::adjustExpenseAmount(double additionalCost) {
    if (additionalCost <= 0.0) {
        throw RepairCompanyException("Additional expense cost must be positive.");
    }
    expenseAmount += additionalCost;
}

bool ExpenseItem::isExceedingBudgetThreshold() const noexcept {
    return expenseAmount > CRITICAL_EXPENSE_ALERT_LIMIT;
}

// ==========================================
// FinancialLedger
// ==========================================

FinancialLedger::FinancialLedger(int id, std::string period)
    : ledgerId(id),
      totalIncomeBalance(0.0),
      totalExpenseBalance(0.0),
      reportingPeriod(std::move(period)) {
    if (reportingPeriod.empty()) {
        throw RepairCompanyException("General ledger reporting period cannot be empty.");
    }
}

int FinancialLedger::getLedgerId() const noexcept {
    return ledgerId;
}

void FinancialLedger::recordIncome(double incomeAmount) {
    if (incomeAmount <= 0.0) {
        throw InsufficientPaymentException("Income amount recorded in the ledger must be positive.");
    }
    totalIncomeBalance += incomeAmount;
}

void FinancialLedger::recordExpense(double expenseAmount) {
    if (expenseAmount <= 0.0) {
        throw RepairCompanyException("Expense amount deducted from the ledger must be positive.");
    }
    totalExpenseBalance += expenseAmount;
}

double FinancialLedger::calculateNetProfit() const noexcept {
    return totalIncomeBalance - totalExpenseBalance;
}

bool FinancialLedger::reconcileLedger() const noexcept {
    const double netProfit = calculateNetProfit();
    return netProfit >= MINIMUM_ACCEPTABLE_PROFIT_MARGIN;
}

// ==========================================
// SalaryRecord
// ==========================================

SalaryRecord::SalaryRecord(int id, double baseRate, Employee* employee)
    : salaryRecordId(id),
      basePayRate(baseRate),
      performanceBonus(0.0),
      legalDeductions(0.0),
      targetEmployee(employee) {
    if (employee == nullptr) {
        throw RepairCompanyException("Salary record must be linked to an employee.");
    }
    if (baseRate <= 0.0) {
        throw InsufficientPaymentException("Employee base pay rate must be positive.");
    }
}

int SalaryRecord::getSalaryRecordId() const noexcept {
    return salaryRecordId;
}

void SalaryRecord::addBonus(double bonusAmount) {
    if (bonusAmount <= 0.0) {
        throw RepairCompanyException("Bonus amount must be positive.");
    }
    const double maximumPermittedBonus = basePayRate * MAXIMUM_BONUS_MULTIPLIER;
    if (bonusAmount > maximumPermittedBonus) {
        throw RepairCompanyException("Bonus amount exceeds the maximum allowable salary limit.");
    }
    performanceBonus += bonusAmount;
}

void SalaryRecord::applyDeduction(double deductionAmount) {
    if (deductionAmount <= 0.0) {
        throw RepairCompanyException("Payroll deduction amount must be positive.");
    }
    legalDeductions += deductionAmount;
}

double SalaryRecord::calculateNetSalary() const noexcept {
    const double grossSalary = basePayRate + performanceBonus;
    const double calculatedTaxAmount = grossSalary * STANDARD_INCOME_TAX_RATE;
    const double payableSalary = (grossSalary - calculatedTaxAmount) - legalDeductions;
    return std::max(0.0, payableSalary);
}