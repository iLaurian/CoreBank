#ifndef OOP_VALIDATOR_H
#define OOP_VALIDATOR_H

#include <string>

namespace Validator {
    void validateIBAN(const std::string& iban);
    void validateDate(const std::string& dateStr);
    void validateAmount(double amount);
}

#endif