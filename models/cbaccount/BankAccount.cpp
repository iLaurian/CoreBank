#include "BankAccount.h"
#include <memory>
#include "../../utils/cbvalidation/Validator.h"
#include "../../utils/cbdate/DateUtils.h"
#include "../../utils/cblogger/Logger.h"
#include "../../utils/cbexception/BankExceptions.h"
#include "../../utils/cbbanking/ReportGenerator.h"

BankAccount::~BankAccount() {
    Logger::info("Account destroyed: " + IBAN);
}

BankAccount::BankAccount(const std::string& iban, double initialBalance, const Currency curr, const std::string& inception)
    : IBAN(iban), balance(initialBalance), currency(curr),
      inceptionDate(inception.empty() ? getCurrentDate() : inception),
      transactionCount(0), transactionCapacity(100) {

    if (!Validator::validateIBAN(iban) || !Validator::validateAmount(initialBalance) || !Validator::validateDate(inceptionDate)) {
        Logger::error("Account initialization failed for IBAN: " + iban);
        throw ValidationError("Invalid bank account initialization data");
    }

    transactions.reserve(transactionCapacity);
    Logger::info("Account created: " + IBAN);
}

BankAccount::BankAccount(const BankAccount& other)
    : IBAN(other.IBAN), balance(other.balance), currency(other.currency), inceptionDate(other.inceptionDate),
      transactions(other.transactions), transactionCount(other.transactionCount), transactionCapacity(other.transactionCapacity) {

    transactions.reserve(transactionCapacity);
    Logger::info("Account copied: " + IBAN);
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
    Logger::info("Account copied: " + IBAN);
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
    Logger::info("Transaction recorded for IBAN: " + IBAN);
}

const Transaction& BankAccount::getTransaction(int index) const {
    if (index < 0 || index >= transactionCount) {
        Logger::error("Transaction index out of range: " + std::to_string(index));
        throw NotFoundError("Transaction index out of range");
    }
    return transactions[static_cast<size_t>(index)];
}

bool BankAccount::tryPay(double amount, const std::string& dateStr) {
    if (amount > balance) {
        Logger::warn("Payment failed due to insufficient funds: " + IBAN);
        return false;
    }

    const Transaction t(PAYMENT, currency, dateStr, amount, IBAN, "BANK");
    balance -= amount;
    addTransaction(t);
    Logger::info("Payment processed: " + IBAN + " amount=" + std::to_string(amount));

    return true;
}

void BankAccount::processDeposit(double amount, const std::string& dateStr) {
    const Transaction t(DEPOSIT, currency, dateStr, amount, "ATM", IBAN);
    balance += amount;
    addTransaction(t);
    Logger::info("Deposit processed: " + IBAN + " amount=" + std::to_string(amount));
}

bool BankAccount::processWithdrawal(double amount, const std::string& dateStr) {
    if (amount > balance) {
        Logger::error("Withdrawal failed due to insufficient funds: " + IBAN);
        return false;
    }

    const Transaction t(WITHDRAWAL, currency, dateStr, amount, IBAN, "ATM");
    balance -= amount;
    addTransaction(t);
    Logger::info("Withdrawal processed: " + IBAN + " amount=" + std::to_string(amount));
    return true;
}

bool BankAccount::processOutgoingTransfer(double amount, const std::string &toIBAN, const std::string &dateStr) {
    if (amount > balance) {
        Logger::error("Outgoing transfer failed due to insufficient funds: " + IBAN + " -> " + toIBAN);
        return false;
    }

    const Transaction t(TRANSFER, currency, dateStr, amount, IBAN, toIBAN);
    balance -= amount;
    addTransaction(t);
    Logger::info("Outgoing transfer processed: " + IBAN + " -> " + toIBAN + " amount=" + std::to_string(amount));
    return true;
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
    Logger::info("Monthly fee applied: " + IBAN + " fee=" + std::to_string(fee));
}

std::ostream& operator<<(std::ostream& os, const BankAccount& account) {
    account.printDetails(os);
    os << "(IBAN: " << account.IBAN << ", Balance: " << account.balance
       << ", Transactions: " << account.transactionCount << ")" << "\n";

    ReportGenerator rpt("TRANSACTION HISTORY");
    os << rpt.generateTable(account.transactions,
        {"Date", "Type", "Amount", "From", "To", "ID"},
        [](const Transaction& t, size_t col) -> std::string {
            switch (col) {
                case 0: return t.getDate();
                case 1: return t.getType();
                case 2: return std::to_string(t.getAmount());
                case 3: return t.getSourceIBAN();
                case 4: return t.getTargetIBAN();
                case 5: return t.getId();
                default: return "";
            }
        });

    return os;
}
