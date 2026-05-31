#include "InvestmentAccount.h"
#include "../../utils/cbdate/DateUtils.h"
#include "../../utils/cblogger/Logger.h"

InvestmentAccount::InvestmentAccount(const std::string& iban, double initialBalance, Currency curr, const std::string& inceptionDate)
    : BankAccount(iban, initialBalance, curr, inceptionDate) {
}

std::unique_ptr<BankAccount> InvestmentAccount::clone() const {
    return std::make_unique<InvestmentAccount>(*this);
}

bool InvestmentAccount::addBond(const Bond& bond, const std::string& dateStr) {
    if (bond.currency != currency) {
        Logger::error("Bond purchase failed: currency mismatch for IBAN " + IBAN);
        return false;
    }
    if (bond.faceValue > balance) {
        Logger::error("Bond purchase failed: insufficient balance for IBAN " + IBAN);
        return false;
    }
    if (!isValidDate(bond.issueDate) || !isValidDate(bond.maturityDate) || !isValidDate(dateStr)) {
        Logger::error("Bond purchase failed: invalid bond dates for IBAN " + IBAN);
        return false;
    }
    if (daysBetween(bond.issueDate, bond.maturityDate) <= 0) {
        Logger::error("Bond purchase failed: maturity before issue for IBAN " + IBAN);
        return false;
    }

    Bond stored = bond;
    stored.lastCouponYear = parseDate(bond.issueDate).year;

    const Transaction t(PAYMENT, currency, dateStr, bond.faceValue, IBAN, "BANK");
    balance -= bond.faceValue;
    addTransaction(t);

    bonds.push_back(stored);
    Logger::info("Bond purchased: " + bond.id + " IBAN=" + IBAN);
    return true;
}

void InvestmentAccount::creditInterest(double amount, const std::string& dateStr) {
    const Transaction t(INTEREST, currency, dateStr, amount, "BANK", IBAN);
    balance += amount;
    addTransaction(t);
}

void InvestmentAccount::creditPrincipal(double amount, const std::string& dateStr) {
    const Transaction t(DEPOSIT, currency, dateStr, amount, "BANK", IBAN);
    balance += amount;
    addTransaction(t);
}

std::vector<Bond>& InvestmentAccount::getBonds() {
    return bonds;
}

const std::vector<Bond>& InvestmentAccount::getBonds() const {
    return bonds;
}

void InvestmentAccount::printDetails(std::ostream& os) const {
    os << "InvestmentAccount";
}

double InvestmentAccount::getNetAssetValue() const {
    double nav = balance;
    for (const auto& bond : bonds) {
        nav += bond.faceValue;
    }
    return nav;
}

double InvestmentAccount::calculateMonthlyFee() const {
    return getNetAssetValue() * 0.015 / 12.0;
}
