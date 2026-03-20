#include "Validator.h"
#include <stdexcept>
#include <cctype>

namespace Validator {
    void validateIBAN(const std::string& iban) {
        if (iban.length() < 15 || iban.length() > 34) {
            throw std::invalid_argument("Invalid IBAN length: " + iban);
        }
        if (!std::isalpha(iban[0]) || !std::isalpha(iban[1])) {
            throw std::invalid_argument("IBAN must start with 2 letters: " + iban);
        }
        if (!std::isdigit(iban[2]) || !std::isdigit(iban[3])) {
            throw std::invalid_argument("IBAN characters 3 and 4 must be digits: " + iban);
        }
        for (const char c : iban) {
            if (!std::isalnum(c)) {
                throw std::invalid_argument("IBAN contains invalid characters: " + iban);
            }
        }
    }

    void validateDate(const std::string& dateStr) {
        if (dateStr.size() != 10 || dateStr[4] != '-' || dateStr[7] != '-') {
            throw std::invalid_argument("Invalid date format: " + dateStr);
        }
    }

    void validateAmount(const double amount) {
        if (amount < 0) {
            throw std::invalid_argument("Amount cannot be negative: " + std::to_string(amount));
        }
    }
}