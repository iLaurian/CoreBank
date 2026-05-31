#include "JsonLoader.h"

#include <fstream>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include "Logger.h"
#include "BankExceptions.h"
#include "DateUtils.h"
#include "Validator.h"
#include "../models/Bank.h"
#include "../models/Client.h"
#include "../models/PersonalAccount.h"
#include "../models/SavingsAccount.h"
#include "../models/RetirementAccount.h"
#include "../models/InvestmentAccount.h"
#include "../models/Bond.h"

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

    Currency parseCurrency(const std::string& code) {
        if (code == "USD") return USD;
        if (code == "EUR") return EUR;
        if (code == "GBP") return GBP;
        if (code == "JPY") return JPY;
        if (code == "CHF") return CHF;
        Logger::error("Unsupported currency code: " + code);
        throw ValidationError("Unsupported currency code: " + code);
    }

    std::unique_ptr<BankAccount> createAccount(const nlohmann::json& item) {
        const std::string type = item.at("type").get<std::string>();
        const std::string iban = item.at("iban").get<std::string>();
        const double balance = item.at("balance").get<double>();
        const std::string currencyCode = item.at("currency").get<std::string>();
        const std::string inceptionDate = item.value("inceptionDate", "");

        if (!Validator::validateIBAN(iban) || !Validator::validateAmount(balance)) {
            throw ValidationError("Invalid account data for IBAN: " + iban);
        }
        if (!inceptionDate.empty() && !Validator::validateDate(inceptionDate)) {
            throw ValidationError("Invalid inception date for IBAN: " + iban);
        }

        const Currency currency = parseCurrency(currencyCode);

        if (type == "personal") {
            return std::make_unique<PersonalAccount>(iban, balance, currency, inceptionDate);
        }
        if (type == "savings") {
            return std::make_unique<SavingsAccount>(iban, balance, currency, inceptionDate);
        }
        if (type == "retirement") {
            const std::string maturity = item.at("maturityDate").get<std::string>();
            if (!Validator::validateDate(maturity)) {
                throw ValidationError("Invalid maturity date for IBAN: " + iban);
            }
            return std::make_unique<RetirementAccount>(iban, balance, currency, maturity, inceptionDate);
        }
        if (type == "investment") {
            return std::make_unique<InvestmentAccount>(iban, balance, currency, inceptionDate);
        }

        throw ValidationError("Unknown account type: " + type);
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

        std::unique_ptr<BankAccount> account = createAccount(item);
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
        bond.currency = parseCurrency(item.at("currency").get<std::string>());
        bond.lastCouponYear = item.value("lastCouponYear", 0);

        investment->addBond(bond, getCurrentDate());
    }

    return bank;
}
