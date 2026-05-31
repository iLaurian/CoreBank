#ifndef OOP_ACCOUNTFACTORY_H
#define OOP_ACCOUNTFACTORY_H

#include <memory>
#include <string>

#include <nlohmann/json.hpp>

#include "BankAccount.h"

namespace AccountFactory {
    std::unique_ptr<BankAccount> createAccount(const nlohmann::json& item);
}

#endif
