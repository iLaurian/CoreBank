#include "BankAccount.h"
#include "../utils/Validator.h"

BankAccount::~BankAccount() = default;

BankAccount::BankAccount(const std::string& iban, double initialBalance, const Currency curr)
    : IBAN(iban), balance(initialBalance), currency(curr), transactionCount(0), transactionCapacity(100) {

    Validator::validateIBAN(iban);
    Validator::validateAmount(initialBalance);

    transactions.reserve(transactionCapacity);
}

BankAccount::BankAccount(const BankAccount& other)
    : IBAN(other.IBAN), balance(other.balance), currency(other.currency), transactions(other.transactions),
      transactionCount(other.transactionCount), transactionCapacity(other.transactionCapacity) {

    transactions.reserve(transactionCapacity);
}

BankAccount& BankAccount::operator=(const BankAccount& other) {
    if (this != &other) {
        IBAN = other.IBAN;
        balance = other.balance;
        currency = other.currency;
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

std::ostream& operator<<(std::ostream& os, const BankAccount& account) {
    os << "BankAccount(IBAN: " << account.IBAN << ", Balance: " << account.balance
       << ", Transactions: " << account.transactionCount << ")" << "\n";

    for (int i = 0; i < account.transactionCount; ++i) {
        os << "  " << account.transactions[static_cast<size_t>(i)] << "\n";
    }

    return os;
}
