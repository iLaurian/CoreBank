#include "BankAccount.h"
#include <memory>
#include "../utils/Validator.h"
#include "../utils/DateUtils.h"

BankAccount::~BankAccount() = default;

BankAccount::BankAccount(const std::string& iban, double initialBalance, const Currency curr, const std::string& inception)
    : IBAN(iban), balance(initialBalance), currency(curr),
      inceptionDate(inception.empty() ? getCurrentDate() : inception),
      transactionCount(0), transactionCapacity(100) {

    Validator::validateIBAN(iban);
    Validator::validateAmount(initialBalance);
    Validator::validateDate(inceptionDate);

    transactions.reserve(transactionCapacity);
}

BankAccount::BankAccount(const BankAccount& other)
    : IBAN(other.IBAN), balance(other.balance), currency(other.currency), inceptionDate(other.inceptionDate),
      transactions(other.transactions), transactionCount(other.transactionCount), transactionCapacity(other.transactionCapacity) {

    transactions.reserve(transactionCapacity);
}

BankAccount& BankAccount::operator=(const BankAccount& other) {
    if (this != &other) {
        IBAN = other.IBAN;
        balance = other.balance;
        currency = other.currency;
        inceptionDate = other.inceptionDate;
        transactions = other.transactions;
        transactionCapacity = other.transactionCapacity;
        transactions.reserve(transactionCapacity);
        transactionCount = other.transactionCount;
    }
    return *this;
}

void BankAccount::resizeTransactions() {
    transactionCapacity *= 2;
    transactions.reserve(transactionCapacity);
}

void BankAccount::printDetails(std::ostream& os) const {
    os << "BankAccount";
}

const std::string& BankAccount::getIBAN() const {
    return IBAN;
}

double BankAccount::getBalance() const {
    return balance;
}

int BankAccount::getTransactionCount() const {
    return transactionCount;
}

Currency BankAccount::getCurrency() const {
    return currency;
}

const std::string& BankAccount::getInceptionDate() const {
    return inceptionDate;
}

void BankAccount::addTransaction(const Transaction& t) {
    if (transactionCount >= transactionCapacity) {
        resizeTransactions();
    }
    transactions.push_back(t);
    ++transactionCount;
}

const Transaction& BankAccount::getTransaction(int index) const {
    if (index < 0 || index >= transactionCount) {
        throw std::out_of_range("Transaction index out of range");
    }
    return transactions[static_cast<size_t>(index)];
}

bool BankAccount::tryPay(double amount, const std::string& dateStr) {
    if (amount > balance) {
        return false;
    }

    const Transaction t(PAYMENT, currency, dateStr, amount, IBAN, "BANK");
    balance -= amount;
    addTransaction(t);

    return true;
}

void BankAccount::processDeposit(double amount, const std::string& dateStr) {
    const Transaction t(DEPOSIT, currency, dateStr, amount, "ATM", IBAN);
    balance += amount;
    addTransaction(t);
}

void BankAccount::processWithdrawal(double amount, const std::string& dateStr) {
    if (amount > balance) {
        throw std::invalid_argument("Insufficient funds for withdrawal");
    }

    const Transaction t(WITHDRAWAL, currency, dateStr, amount, IBAN, "ATM");
    balance -= amount;
    addTransaction(t);
}

void BankAccount::processOutgoingTransfer(double amount, const std::string &toIBAN, const std::string &dateStr) {
    if (amount > balance) {
        throw std::invalid_argument("Insufficient funds for transfer");
    }

    const Transaction t(TRANSFER, currency, dateStr, amount, IBAN, toIBAN);
    balance -= amount;
    addTransaction(t);
}

void BankAccount::processIncomingTransfer(double amount, const std::string& fromIBAN, const std::string& dateStr) {
    const Transaction t(TRANSFER, currency, dateStr, amount, fromIBAN, IBAN);
    balance += amount;
    addTransaction(t);
}

void BankAccount::applyMonthlyFee(const std::string& dateStr) {
    const double fee = calculateMonthlyFee();
    if (fee <= 0.0) {
        return;
    }

    const Transaction t(FEE, currency, dateStr, fee, IBAN, "BANK");
    balance -= fee;
    addTransaction(t);
}

std::ostream& operator<<(std::ostream& os, const BankAccount& account) {
    account.printDetails(os);
    os << "(IBAN: " << account.IBAN << ", Balance: " << account.balance
       << ", Transactions: " << account.transactionCount << ")" << "\n";

    for (int i = 0; i < account.transactionCount; ++i) {
        os << "  " << account.transactions[static_cast<size_t>(i)] << "\n";
    }

    return os;
}
