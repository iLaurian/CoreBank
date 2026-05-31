#include "Validator.h"
#include <stdexcept>
#include <cctype>
#include "../cbdate/DateUtils.h"
#include "../cblogger/Logger.h"

namespace Validator {
    bool validateIBAN(const std::string& iban) {
        if (iban.length() < 15 || iban.length() > 34) {
            Logger::error("Invalid IBAN length: " + iban);
            return false;
        }
        if (!std::isalpha(iban[0]) || !std::isalpha(iban[1])) {
            Logger::error("IBAN must start with 2 letters: " + iban);
            return false;
        }
        if (!std::isdigit(iban[2]) || !std::isdigit(iban[3])) {
            Logger::error("IBAN characters 3 and 4 must be digits: " + iban);
            return false;
        }
        for (const char c : iban) {
            if (!std::isalnum(c)) {
                Logger::error("IBAN contains invalid characters: " + iban);
                return false;
            }
        }
        return true;
    }

    bool validateDate(const std::string& dateStr) {
        if (!isValidDate(dateStr)) {
            Logger::error("Invalid date format: " + dateStr);
            return false;
        }
        return true;
    }

    bool validateAmount(const double amount) {
        if (amount < 0) {
            Logger::error("Amount cannot be negative: " + std::to_string(amount));
            return false;
        }
        return true;
    }
}
