#include "JsonLoader.h"

#include <fstream>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include "../cblogger/Logger.h"
#include "../cbexception/BankExceptions.h"
#include "../cbdate/DateUtils.h"
#include "../cbvalidation/Validator.h"
#include "../cbcurrency/CurrencyConverter.h"
#include "../../models/cbaccount/AccountFactory.h"
#include "../../models/cbbank/Bank.h"
#include "../../models/cbclient/Client.h"
#include "../../models/cbaccount/InvestmentAccount.h"
#include "../../models/cbinstrument/Bond.h"

namespace {
    nlohmann::json loadJsonFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            Logger::error("Failed to open JSON file: " + path);
            throw NotFoundError("Cannot open JSON file: " + path);
        }
        nlohmann::json data;
        file >> data;
        return data;
    }

}

Bank& JsonLoader::loadBankData(const std::string& bankPath,
                               const std::string& clientsPath,
                               const std::string& accountsPath,
                               const std::string& bondsPath,
                               const std::string& loansPath) {
    const nlohmann::json bankJson = loadJsonFile(bankPath);
    const nlohmann::json clientsJson = loadJsonFile(clientsPath);
    const nlohmann::json accountsJson = loadJsonFile(accountsPath);
    const nlohmann::json bondsJson = loadJsonFile(bondsPath);
    (void)loansPath;

    if (!bankJson.contains("name") || !bankJson.contains("swiftCode")) {
        throw ValidationError("bank.json missing required fields");
    }

    Bank::initialize(bankJson.at("name").get<std::string>(),
                     bankJson.at("swiftCode").get<std::string>());
    Bank& bank = Bank::instance();

    std::unordered_map<std::string, Client*> clientIndex;
    for (const auto& item : clientsJson) {
        const std::string cnp = item.at("cnp").get<std::string>();
        const std::string name = item.at("name").get<std::string>();
        const std::string address = item.at("address").get<std::string>();
        const double income = item.at("monthlyIncome").get<double>();

        Client* client = bank.registerClient(cnp, name, address, income);
        clientIndex[cnp] = client;
    }

    std::unordered_map<std::string, BankAccount*> accountIndex;
    for (const auto& item : accountsJson) {
        const std::string ownerCnp = item.at("ownerCnp").get<std::string>();
        auto it = clientIndex.find(ownerCnp);
        if (it == clientIndex.end()) {
            throw ValidationError("Account owner not found: " + ownerCnp);
        }

        std::unique_ptr<BankAccount> account = AccountFactory::createAccount(item);
        const std::string iban = account->getIBAN();
        BankAccount* raw = account.get();
        it->second->addBankAccount(std::move(account));
        accountIndex[iban] = raw;
    }

    for (const auto& item : bondsJson) {
        const std::string ownerIban = item.at("ownerIban").get<std::string>();
        auto it = accountIndex.find(ownerIban);
        if (it == accountIndex.end()) {
            throw ValidationError("Bond owner account not found: " + ownerIban);
        }
        auto* investment = dynamic_cast<InvestmentAccount*>(it->second);
        if (!investment) {
            throw ValidationError("Bond owner is not investment account: " + ownerIban);
        }

        Bond bond;
        bond.id = item.at("id").get<std::string>();
        bond.faceValue = item.at("faceValue").get<double>();
        bond.annualCouponRate = item.at("annualCouponRate").get<double>();
        bond.issueDate = item.at("issueDate").get<std::string>();
        bond.maturityDate = item.at("maturityDate").get<std::string>();
        bond.currency = CurrencyConverter::parseCurrency(item.at("currency").get<std::string>());
        bond.lastCouponYear = item.value("lastCouponYear", 0);

        investment->addBond(bond, getCurrentDate());
    }

    return bank;
}
