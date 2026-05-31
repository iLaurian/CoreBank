#include "Transaction.h"
#include "../../utils/cbvalidation/Validator.h"
#include "../../utils/cblogger/Logger.h"
#include "../../utils/cbexception/BankExceptions.h"
#include <random>
#include <sstream>
#include <stdexcept>

std::string Transaction::generateUniqueId() {
    // static std::random_device rd;
    // static std::mt19937 gen(rd());
    // static std::uniform_int_distribution<> dis(0, 15);
    //
    // std::stringstream ss;
    // for (int i = 0; i < 8; ++i) {
    //     ss << std::hex << dis(gen);
    // }
    // return ss.str();
    static int counter = 100000;
    return std::to_string(++counter);
}

bool Transaction::validateTransactionLogic(TransactionType type, const std::string& srcIBAN, const std::string& tgtIBAN) {
    if (type == DEPOSIT) {
        if (srcIBAN != "ATM" && srcIBAN != "BANK") {
            Logger::error("DEPOSIT transactions must have source IBAN as 'ATM' or 'BANK'");
            return false;
        }
        return Validator::validateIBAN(tgtIBAN);
    }
    if (type == WITHDRAWAL) {
        if (tgtIBAN != "ATM") {
            Logger::error("WITHDRAWAL transactions must have target IBAN as 'ATM'");
            return false;
        }
        return Validator::validateIBAN(srcIBAN);
    }
    if (type == TRANSFER) {
        if (!Validator::validateIBAN(srcIBAN) || !Validator::validateIBAN(tgtIBAN)) {
            return false;
        }
        if (srcIBAN == tgtIBAN) {
            Logger::error("Source and target IBAN cannot be the same for TRANSFER or PAYMENT");
            return false;
        }
        return true;
    }
    if (type == PAYMENT) {
        if (srcIBAN != "BANK") {
            if (!Validator::validateIBAN(srcIBAN)) {
                return false;
            }
        }
        if (tgtIBAN != "BANK") {
            if (!Validator::validateIBAN(tgtIBAN)) {
                return false;
            }
        }
        if (srcIBAN == tgtIBAN) {
            Logger::error("Source and target IBAN cannot be the same for TRANSFER or PAYMENT");
            return false;
        }
        return true;
    }
    if (type == INTEREST) {
        if (srcIBAN != "BANK") {
            Logger::error("INTEREST transactions must have source IBAN as 'BANK'");
            return false;
        }
        return Validator::validateIBAN(tgtIBAN);
    }
    if (type == FEE) {
        if (!Validator::validateIBAN(srcIBAN)) {
            return false;
        }
        if (tgtIBAN != "BANK") {
            Logger::error("FEE transactions must have target IBAN as 'BANK'");
            return false;
        }
        return true;
    }
    return false;
}

Transaction::Transaction(const TransactionType tType, Currency curr,
                         const std::string& dateStr, double amt,
                         const std::string& srcIBAN, const std::string& tgtIBAN)
    : id(generateUniqueId()),
      date(dateStr),
      amount(amt),
      type(tType),
      currency(curr),
      sourceIBAN(srcIBAN),
      targetIBAN(tgtIBAN) {
    if (!Validator::validateDate(dateStr) || !Validator::validateAmount(amt)) {
        Logger::error("Invalid transaction data");
        throw ValidationError("Invalid transaction data");
    }
    if (!validateTransactionLogic(type, sourceIBAN, targetIBAN)) {
        Logger::error("Invalid transaction logic");
        throw ValidationError("Invalid transaction logic");
    }
    Logger::info("Transaction created: " + getType() + " " + std::to_string(amount) +
                 " " + getCurrency() + " From: " + sourceIBAN + " To: " + targetIBAN);
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
        case INTEREST: return "INTEREST";
        case FEE: return "FEE";
        default: return "UNKNOWN";
    }
}

std::string Transaction::getCurrency() const {
    return currencyToString(currency);
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
