#include "SavingsAccount.h"
#include <stdexcept>
#include "../utils/DateUtils.h"
#include "../utils/CurrencyConverter.h"
#include "../utils/InterestAccrual.h"
#include "../utils/Logger.h"

SavingsAccount::SavingsAccount(const std::string& iban, double initialBalance, Currency curr, const std::string& inceptionDate)
    : BankAccount(iban, initialBalance, curr, inceptionDate), lastInterestDate(getInceptionDate()) {
}

std::unique_ptr<BankAccount> SavingsAccount::clone() const {
    return std::make_unique<SavingsAccount>(*this);
}

bool SavingsAccount::processWithdrawal(double amount, const std::string& dateStr) {
    if (amount > balance) {
        Logger::error("Savings withdrawal failed due to insufficient funds: " + IBAN);
        return false;
    }
    if (balance - amount < MinimumBalance) {
        Logger::error("Savings withdrawal failed due to minimum balance: " + IBAN);
        return false;
    }

    return BankAccount::processWithdrawal(amount, dateStr);
}

bool SavingsAccount::processOutgoingTransfer(double amount, const std::string& toIBAN, const std::string& dateStr) {
    if (amount > balance) {
        Logger::error("Savings transfer failed due to insufficient funds: " + IBAN + " -> " + toIBAN);
        return false;
    }
    if (balance - amount < MinimumBalance) {
        Logger::error("Savings transfer failed due to minimum balance: " + IBAN + " -> " + toIBAN);
        return false;
    }

    return BankAccount::processOutgoingTransfer(amount, toIBAN, dateStr);
}

double SavingsAccount::calculateAccruedInterest(const std::string& upToDate) const {
    return InterestAccrual::calculate(transactions, IBAN, lastInterestDate, upToDate, AnnualInterestRate, balance);
}

void SavingsAccount::applyInterestIfDue(const std::string& dateStr) {
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

void SavingsAccount::closeAndTransferTo(BankAccount& target, const std::string& dateStr) {
    const double fullAmount = balance;
    if (fullAmount <= 0.0) {
        return;
    }

    const Transaction outgoing(TRANSFER, currency, dateStr, fullAmount, IBAN, target.getIBAN());
    balance -= fullAmount;
    addTransaction(outgoing);

    const double convertedAmount = CurrencyConverter::convert(fullAmount, currency, target.getCurrency());
    target.processIncomingTransfer(convertedAmount, IBAN, dateStr);
}

void SavingsAccount::printDetails(std::ostream& os) const {
    os << "SavingsAccount";
}

double SavingsAccount::calculateMonthlyFee() const {
    return balance * 0.01 / 12.0;
}
