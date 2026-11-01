#pragma once

#include <string>

// Forward declarations of related domain classes
class Client;
class WorkOrder;
class Employee;

// ==========================================
// LOYALTY CARD
// ==========================================

class LoyaltyCard {
private:
    static constexpr double INITIAL_DISCOUNT_PERCENT = 3.0;
    static constexpr double MAXIMUM_TIER_DISCOUNT_PERCENT = 15.0;
    static constexpr double TIER_UPGRADE_SPENDING_THRESHOLD = 1000.0;

    int cardId;
    double currentDiscountPercent;
    double totalSpentAccumulated;
    bool isActiveCard;

public:
    explicit LoyaltyCard(int id);

    int getCardId() const noexcept;
    double getDiscountPercent() const noexcept;
    void accumulateSpending(double paymentAmount);
    bool isEligibleForTierUpgrade() const noexcept;
    void upgradeDiscountTier(double discountIncrement);
};

// ==========================================
// DISCOUNT POLICY
// ==========================================

class DiscountPolicy {
private:
    static constexpr double MAXIMUM_ALLOWED_DISCOUNT_CAP = 25.0;
    static constexpr double MINIMUM_APPLICABLE_DISCOUNT = 0.0;

    int policyId;
    std::string policyName;
    double maximumDiscountCap;

public:
    DiscountPolicy(int id, std::string name, double maxCap);

    void validateDiscountRate(double requestedDiscountRate) const;
    double calculateAdjustedDiscount(double requestedRate, const LoyaltyCard* clientCard) const;
    bool isPolicyApplicable(double orderTotalAmount) const noexcept;
};

// ==========================================
// INVOICE
// ==========================================

class Invoice {
private:
    static constexpr double STANDARD_TAX_RATE = 0.20; // VAT 20%
    static constexpr double MINIMUM_PAYABLE_THRESHOLD = 0.01;

    int invoiceId;
    double baseLaborAmount;
    double partsCostAmount;
    double appliedDiscountPercentage;
    double calculatedTaxAmount;
    bool isInvoiceSettled;

    Client* billedClient;
    WorkOrder* targetWorkOrder;

public:
    Invoice(int id, double laborCost, double partsCost, Client* client, WorkOrder* order);

    int getInvoiceId() const noexcept;
    void applyDiscountRate(double discountRate);
    double calculateFinalPayableAmount() const noexcept;
    void markAsSettled();
    bool isSettled() const noexcept;

    bool operator==(const Invoice& other) const noexcept;
};

// ==========================================
// PAYMENT TRANSACTION
// ==========================================

class PaymentTransaction {
protected:
    static constexpr double MINIMUM_TRANSACTION_AMOUNT = 0.01;

    int transactionId;
    std::string transactionDate;
    double transferredAmount;
    bool isSuccessful;
    Invoice* associatedInvoice;

public:
    PaymentTransaction(int id, std::string date, double amount, Invoice* invoice);
    virtual ~PaymentTransaction() = default;

    int getTransactionId() const noexcept;
    void processTransaction();
    void refundTransaction();
    void verifyPaymentIntegrity() const;

    bool operator==(const PaymentTransaction& other) const noexcept;
};

// ==========================================
// CASH PAYMENT
// ==========================================

class CashPayment : public PaymentTransaction {
private:
    static constexpr double CHANGE_EPSILON_TOLERANCE = 0.001;

    double cashTendered;
    double changeDueAmount;
    std::string fiscalReceiptNumber;

public:
    CashPayment(int id, std::string date, double amount, Invoice* invoice,
                double tendered, std::string receiptNum);

    double calculateChangeAmount();
    void printFiscalReceiptDetails() const;
};

// ==========================================
// CARD PAYMENT (CARD / BANK TERMINAL)
// ==========================================

class CardPayment : public PaymentTransaction {
private:
    static constexpr size_t REQUIRED_AUTH_CODE_LENGTH = 6;

    std::string authorizationCode;
    std::string terminalIdentifier;
    std::string maskedCardNumber;
    bool isContactless;

public:
    CardPayment(int id, std::string date, double amount, Invoice* invoice,
                std::string authCode, std::string terminalId, std::string maskedCard, bool contactless);

    void authorizeTerminalPayment();
    void validateAuthorizationCode() const;
};

// ==========================================
// EXPENSE ITEM
// ==========================================

class ExpenseItem {
private:
    static constexpr double CRITICAL_EXPENSE_ALERT_LIMIT = 5000.0;

    int expenseId;
    std::string categoryName;
    double expenseAmount;
    std::string expenseDate;

public:
    ExpenseItem(int id, std::string category, double amount, std::string date);

    int getExpenseId() const noexcept;
    void adjustExpenseAmount(double additionalCost);
    bool isExceedingBudgetThreshold() const noexcept;
};

// ==========================================
// FINANCIAL LEDGER
// ==========================================

class FinancialLedger {
private:
    static constexpr double MINIMUM_ACCEPTABLE_PROFIT_MARGIN = 0.0;

    int ledgerId;
    double totalIncomeBalance;
    double totalExpenseBalance;
    std::string reportingPeriod;

public:
    FinancialLedger(int id, std::string period);

    int getLedgerId() const noexcept;
    void recordIncome(double incomeAmount);
    void recordExpense(double expenseAmount);
    double calculateNetProfit() const noexcept;
    bool reconcileLedger() const noexcept;
};

// ==========================================
// SALARY RECORD
// ==========================================

class SalaryRecord {
private:
    static constexpr double STANDARD_INCOME_TAX_RATE = 0.13; // Income Tax 13%
    static constexpr double MAXIMUM_BONUS_MULTIPLIER = 2.0;

    int salaryRecordId;
    double basePayRate;
    double performanceBonus;
    double legalDeductions;
    Employee* targetEmployee;

public:
    SalaryRecord(int id, double baseRate, Employee* employee);

    int getSalaryRecordId() const noexcept;
    void addBonus(double bonusAmount);
    void applyDeduction(double deductionAmount);
    double calculateNetSalary() const noexcept;
};