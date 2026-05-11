#ifndef OOP_BANKEXCEPTIONS_H
#define OOP_BANKEXCEPTIONS_H

#include <stdexcept>
#include <string>

class BankException : public std::runtime_error {
public:
    explicit BankException(const std::string& message) : std::runtime_error(message) {}
};

class ValidationError : public BankException {
public:
    explicit ValidationError(const std::string& message) : BankException(message) {}
};

class NotFoundError : public BankException {
public:
    explicit NotFoundError(const std::string& message) : BankException(message) {}
};

class ConfigurationError : public BankException {
public:
    explicit ConfigurationError(const std::string& message) : BankException(message) {}
};

#endif
