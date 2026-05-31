#ifndef OOP_JSONHELPERS_H
#define OOP_JSONHELPERS_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "../../models/cbclient/Client.h"
#include "../../models/cbaccount/BankAccount.h"
#include "../../models/cbaccount/PersonalAccount.h"
#include "../../models/cbaccount/SavingsAccount.h"
#include "../../models/cbaccount/RetirementAccount.h"
#include "../../models/cbaccount/InvestmentAccount.h"
#include "../../models/cbinstrument/Bond.h"
#include "../../models/cbloan/Loan.h"
#include "../../server/ApiTypes.h"

inline AccountKind detectAccountKind(const BankAccount& account) {
    if (dynamic_cast<const PersonalAccount*>(&account)) return AccountKind::Personal;
    if (dynamic_cast<const SavingsAccount*>(&account)) return AccountKind::Savings;
    if (dynamic_cast<const RetirementAccount*>(&account)) return AccountKind::Retirement;
    if (dynamic_cast<const InvestmentAccount*>(&account)) return AccountKind::Investment;
    return AccountKind::Personal;
}

inline nlohmann::json makeError(const std::string& code, const std::string& message) {
    return nlohmann::json{{"error", { {"code", code}, {"message", message} }}};
}

inline nlohmann::json clientToJson(const Client& client) {
    return {
        {"cnp", client.getCNP()},
        {"name", client.getName()},
        {"address", client.getAddress()},
        {"monthlyIncome", client.getMonthlyIncome()},
        {"creditScore", client.getCreditScore()}
    };
}

inline nlohmann::json accountToJson(const BankAccount& account) {
    return {
        {"type", accountKindToString(detectAccountKind(account))},
        {"iban", account.getIBAN()},
        {"currency", currencyToString(account.getCurrency())},
        {"balance", account.getBalance()},
        {"inceptionDate", account.getInceptionDate()}
    };
}

inline nlohmann::json transactionToJson(const Transaction& t) {
    return {
        {"id", t.getId()},
        {"date", t.getDate()},
        {"type", t.getType()},
        {"amount", t.getAmount()},
        {"currency", t.getCurrency()},
        {"from", t.getSourceIBAN()},
        {"to", t.getTargetIBAN()}
    };
}

inline nlohmann::json bondToJson(const Bond& bond) {
    return {
        {"id", bond.id},
        {"faceValue", bond.faceValue},
        {"annualCouponRate", bond.annualCouponRate},
        {"issueDate", bond.issueDate},
        {"maturityDate", bond.maturityDate},
        {"currency", currencyToString(bond.currency)},
        {"lastCouponYear", bond.lastCouponYear}
    };
}

inline nlohmann::json loanToJson(const Loan& loan) {
    std::string status = "ACTIVE";
    if (loan.status == PAID) status = "PAID";
    if (loan.status == OVERDUE) status = "OVERDUE";
    return {
        {"id", loan.id},
        {"principal", loan.principal},
        {"annualInterestRate", loan.annualInterestRate},
        {"termMonths", loan.termMonths},
        {"startDate", loan.startDate},
        {"nextDueDate", loan.nextDueDate},
        {"paymentIBAN", loan.paymentIBAN},
        {"currency", currencyToString(loan.currency)},
        {"monthlyPayment", loan.monthlyPayment},
        {"remainingBalance", loan.remainingBalance},
        {"status", status},
        {"missedPayments", loan.missedPayments}
    };
}

#endif
