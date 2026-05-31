#ifndef OOP_VALIDATOR_H
#define OOP_VALIDATOR_H

#include <string>

namespace Validator {
    bool validateIBAN(const std::string& iban);
    bool validateDate(const std::string& dateStr);
    bool validateAmount(double amount);
}

#endif
