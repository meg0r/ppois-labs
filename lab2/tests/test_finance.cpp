#include "Finance.hpp"
#include "Exceptions.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

static void testLoyaltyCardTierUpgrade() {
    constexpr int CARD_ID = 101;
    constexpr double SPENT_AMOUNT = 1500.0;
    constexpr double TIER_INCREMENT = 2.0;

    LoyaltyCard card(CARD_ID);
    assert(!card.isEligibleForTierUpgrade());

    card.accumulateSpending(SPENT_AMOUNT);
    assert(card.isEligibleForTierUpgrade());

    bool upgradeSucceeded = true;
    try {
        card.upgradeDiscountTier(TIER_INCREMENT);
    } catch (const std::exception&) {
        upgradeSucceeded = false;
    }
    assert(upgradeSucceeded);
}

static void testDiscountPolicyRejectsExcessiveRate() {
    constexpr int POLICY_ID = 201;
    constexpr double MAX_DISCOUNT_CAP = 20.0;
    constexpr double EXCESSIVE_DISCOUNT = 30.0;

    DiscountPolicy policy(POLICY_ID, "Standard Policy", MAX_DISCOUNT_CAP);

    bool exceptionThrown = false;
    try {
        policy.validateDiscountRate(EXCESSIVE_DISCOUNT);
    } catch (const InvalidDiscountException&) {
        exceptionThrown = true;
    } catch (const RepairCompanyException&) {
        exceptionThrown = true;
    }
    assert(exceptionThrown);
}

static void testExpenseItemThresholdDetection() {
    constexpr int EXPENSE_ID = 301;
    constexpr double CRITICAL_AMOUNT = 6000.0;
    constexpr double NORMAL_AMOUNT = 1500.0;

    ExpenseItem heavyExpense(EXPENSE_ID, "Equipment Replacement", CRITICAL_AMOUNT, "2026-03-01");
    assert(heavyExpense.isExceedingBudgetThreshold());

    ExpenseItem normalExpense(EXPENSE_ID + 1, "Stationery", NORMAL_AMOUNT, "2026-03-01");
    assert(!normalExpense.isExceedingBudgetThreshold());
}

static void testLedgerProfitAndReconciliation() {
    constexpr int LEDGER_ID = 401;
    constexpr double REVENUE_AMOUNT = 3000.0;
    constexpr double EXPENSE_AMOUNT = 1200.0;
    constexpr double EXPECTED_PROFIT = 1800.0;
    constexpr double EPSILON = 0.001;

    FinancialLedger ledger(LEDGER_ID, "2026-Q1");
    ledger.recordIncome(REVENUE_AMOUNT);
    ledger.recordExpense(EXPENSE_AMOUNT);

    const double actualProfit = ledger.calculateNetProfit();
    assert(std::abs(actualProfit - EXPECTED_PROFIT) < EPSILON);
    assert(ledger.reconcileLedger());
}

void runFinanceTests() {
    testLoyaltyCardTierUpgrade();
    testDiscountPolicyRejectsExcessiveRate();
    testExpenseItemThresholdDetection();
    testLedgerProfitAndReconciliation();
    std::cout << " [PASS] Finance module\n";
}