#ifndef OOP_LOAN_H
#define OOP_LOAN_H

#include <string>
#include <optional>
#include "../cbtransaction/Transaction.h"

enum LoanStatus {
    ACTIVE, PAID, OVERDUE
};

struct Loan {
    std::string id;
    double principal;
    double annualInterestRate;
    int termMonths;
    std::string startDate;
    std::string nextDueDate;
    std::string paymentIBAN;
    Currency currency;
    double monthlyPayment;
    double remainingBalance;
    LoanStatus status;
    int missedPayments;

    static std::string generateId();
};

struct LoanRequestResult {
    bool approved;
    std::string reason;
    std::optional<Loan> loan;
};

#endif
