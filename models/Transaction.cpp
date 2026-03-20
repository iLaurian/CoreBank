#include "Transaction.h"
#include "../utils/Validator.h"
#include <random>
#include <sstream>
#include <stdexcept>

std::string Transaction::generateUniqueId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    for (int i = 0; i < 8; ++i) {
        ss << std::hex << dis(gen);
    }
    return ss.str();
}

TransactionType Transaction::validateType(const std::string& typeStr) {
    if (typeStr == "DEPOSIT") return DEPOSIT;
    if (typeStr == "WITHDRAWAL") return WITHDRAWAL;
    if (typeStr == "TRANSFER") return TRANSFER;
    if (typeStr == "PAYMENT") return PAYMENT;
    throw std::invalid_argument("Invalid transaction type: " + typeStr);
}

Currency Transaction::validateCurrency(const std::string& currencyStr) {
    if (currencyStr == "USD") return USD;
    if (currencyStr == "EUR") return EUR;
    if (currencyStr == "GBP") return GBP;
    if (currencyStr == "JPY") return JPY;
    throw std::invalid_argument("Invalid currency: " + currencyStr);
}

void Transaction::validateTransactionLogic(TransactionType type, const std::string& srcIBAN, const std::string& tgtIBAN) {
    if (type == DEPOSIT) {
        if (srcIBAN != "ATM") throw std::invalid_argument("DEPOSIT transactions must have source IBAN as 'ATM'");
        Validator::validateIBAN(tgtIBAN);
    }
    if (type == WITHDRAWAL) {
        if (tgtIBAN != "ATM") throw std::invalid_argument("WITHDRAWAL transactions must have target IBAN as 'ATM'");
        Validator::validateIBAN(srcIBAN);
    }
    if (type == TRANSFER || type == PAYMENT) {
        Validator::validateIBAN(srcIBAN);
        Validator::validateIBAN(tgtIBAN);
        if (srcIBAN == tgtIBAN) {
            throw std::invalid_argument("Source and target IBAN cannot be the same for TRANSFER or PAYMENT");
        }
    }
}

Transaction::Transaction(const std::string& typeStr, const std::string& currencyStr,
                         const std::string& dateStr, double amt,
                         const std::string& srcIBAN, const std::string& tgtIBAN)
    : id(generateUniqueId()),
      date(dateStr),
      amount(amt),
      type(validateType(typeStr)),
      currency(validateCurrency(currencyStr)),
      sourceIBAN(srcIBAN),
      targetIBAN(tgtIBAN) {
    Validator::validateDate(dateStr);
    Validator::validateAmount(amt);
    validateTransactionLogic(type, sourceIBAN, targetIBAN);
}

const std::string& Transaction::getId() const { return id; }
const std::string& Transaction::getDate() const { return date; }
double Transaction::getAmount() const { return amount; }

std::string Transaction::getType() const {
    switch (type) {
        case DEPOSIT: return "DEPOSIT";
        case WITHDRAWAL: return "WITHDRAWAL";
        case TRANSFER: return "TRANSFER";
        case PAYMENT: return "PAYMENT";
        default: return "UNKNOWN";
    }
}

std::string Transaction::getCurrency() const {
    switch (currency) {
        case USD: return "USD";
        case EUR: return "EUR";
        case GBP: return "GBP";
        case JPY: return "JPY";
        default: return "UNKNOWN";
    }
}

const std::string& Transaction::getSourceIBAN() const { return sourceIBAN; }
const std::string& Transaction::getTargetIBAN() const { return targetIBAN; }

std::ostream& operator<<(std::ostream& os, const Transaction& t) {
    os << "[" << t.getDate() << "] " << t.getType()
       << " | " << t.getAmount() << " " << t.getCurrency()
       << " | From: " << t.getSourceIBAN() << " To: " << t.getTargetIBAN()
       << " (ID: " << t.getId() << ")";
    return os;
}