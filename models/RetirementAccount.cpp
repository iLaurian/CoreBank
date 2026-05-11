#include "RetirementAccount.h"
#include <stdexcept>
#include "../utils/DateUtils.h"
#include "../utils/InterestAccrual.h"

RetirementAccount::RetirementAccount(const std::string& iban, double initialBalance, Currency curr, const std::string& maturityDate, const std::string& inceptionDate)
    : BankAccount(iban, initialBalance, curr, inceptionDate), maturityDate(maturityDate), lastInterestDate(getInceptionDate()) {
}

std::unique_ptr<BankAccount> RetirementAccount::clone() const {
    return std::make_unique<RetirementAccount>(*this);
}

bool RetirementAccount::isMaturityReached(const std::string& dateStr) const {
    return dateStr >= maturityDate;
}

double RetirementAccount::calculateAccruedInterest(const std::string& upToDate) const {
    return InterestAccrual::calculate(transactions, IBAN, lastInterestDate, upToDate, AnnualInterestRate, balance);
}

void RetirementAccount::processWithdrawal(double amount, const std::string& dateStr) {
    if (amount > balance) {
        throw std::invalid_argument("Insufficient funds for withdrawal");
    }

    if (!isMaturityReached(dateStr)) {
        const double penalty = amount * EarlyWithdrawalPenaltyRate;
        if (amount + penalty > balance) {
            throw std::invalid_argument("Insufficient funds for retirement withdrawal penalty");
        }
        BankAccount::processWithdrawal(amount + penalty, dateStr);
        return;
    }

    BankAccount::processWithdrawal(amount, dateStr);
}

void RetirementAccount::processOutgoingTransfer(double amount, const std::string& toIBAN, const std::string& dateStr) {
    if (amount > balance) {
        throw std::invalid_argument("Insufficient funds for transfer");
    }

    if (!isMaturityReached(dateStr)) {
        const double penalty = amount * EarlyWithdrawalPenaltyRate;
        if (amount + penalty > balance) {
            throw std::invalid_argument("Insufficient funds for retirement transfer penalty");
        }
        BankAccount::processOutgoingTransfer(amount + penalty, toIBAN, dateStr);
        return;
    }

    BankAccount::processOutgoingTransfer(amount, toIBAN, dateStr);
}

void RetirementAccount::applyInterestIfDue(const std::string& dateStr) {
    if (daysBetween(lastInterestDate, dateStr) < 30) {
        return;
    }

    const double interest = calculateAccruedInterest(dateStr);
    if (interest > 0.0) {
        const Transaction t(INTEREST, currency, dateStr, interest, "BANK", IBAN);
        balance += interest;
        addTransaction(t);
    }

    lastInterestDate = dateStr;
}

void RetirementAccount::printDetails(std::ostream& os) const {
    os << "RetirementAccount";
}

double RetirementAccount::calculateMonthlyFee() const {
    return 0.0;
}
