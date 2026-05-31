#include "InterestAccrual.h"
#include <algorithm>
#include "../cbdate/DateUtils.h"

static double transactionDelta(const Transaction& transaction, const std::string& iban) {
    const std::string& type = transaction.getType();
    if (type == "DEPOSIT" || type == "INTEREST") {
        return transaction.getAmount();
    }
    if (type == "WITHDRAWAL") {
        return -transaction.getAmount();
    }
    if (type == "TRANSFER" || type == "PAYMENT") {
        return transaction.getTargetIBAN() == iban ? transaction.getAmount() : -transaction.getAmount();
    }
    return 0.0;
}

double InterestAccrual::calculate(const std::vector<Transaction>& transactions,
                                  const std::string& iban,
                                  const std::string& fromDate,
                                  const std::string& toDate,
                                  double annualRate,
                                  double currentBalance) {
    if (daysBetween(fromDate, toDate) <= 0) {
        return 0.0;
    }

    std::vector<const Transaction*> periodTransactions;
    periodTransactions.reserve(transactions.size());
    for (const auto &transaction : transactions) {
        if (transaction.getDate() > fromDate && transaction.getDate() <= toDate) {
            periodTransactions.push_back(&transaction);
        }
    }

    std::sort(periodTransactions.begin(), periodTransactions.end(),
              [](const Transaction* lhs, const Transaction* rhs) {
                  return lhs->getDate() < rhs->getDate();
              });

    double runningBalance = currentBalance;
    for (const auto &transaction : transactions) {
        if (transaction.getDate() > toDate) {
            runningBalance -= transactionDelta(transaction, iban);
        }
    }
    for (const Transaction* transaction : periodTransactions) {
        runningBalance -= transactionDelta(*transaction, iban);
    }

    double accruedInterest = 0.0;
    std::string periodStart = fromDate;
    for (const Transaction* transaction : periodTransactions) {
        const std::string& transactionDate = transaction->getDate();
        const int days = daysBetween(periodStart, transactionDate);
        if (days > 0) {
            accruedInterest += runningBalance * (annualRate / 365.0) * days;
        }

        runningBalance += transactionDelta(*transaction, iban);
        periodStart = transactionDate;
    }

    const int remainingDays = daysBetween(periodStart, toDate);
    if (remainingDays > 0) {
        accruedInterest += runningBalance * (annualRate / 365.0) * remainingDays;
    }

    return accruedInterest;
}
