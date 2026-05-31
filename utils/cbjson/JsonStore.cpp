#include "JsonStore.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include "../cbexception/BankExceptions.h"
#include "../cblogger/Logger.h"
#include "../../models/cbaccount/PersonalAccount.h"
#include "../../models/cbaccount/SavingsAccount.h"
#include "../../models/cbaccount/RetirementAccount.h"
#include "../../models/cbaccount/InvestmentAccount.h"
#include "../../server/ApiTypes.h"

JsonStore::JsonStore(std::string bankFile,
                     std::string clientsFile,
                     std::string accountsFile,
                     std::string bondsFile,
                     std::string loansFile)
    : bankPath(std::move(bankFile)),
      clientsPath(std::move(clientsFile)),
      accountsPath(std::move(accountsFile)),
      bondsPath(std::move(bondsFile)),
      loansPath(std::move(loansFile)) {
}

static void writeJsonFile(const std::string& path, const nlohmann::json& data) {
    std::ofstream file(path);
    if (!file.is_open()) {
        Logger::error("Failed to write JSON file: " + path);
        throw ConfigurationError("Cannot write JSON file: " + path);
    }
    file << data.dump(2);
}

static std::string currencyToCode(Currency currency) {
    return currencyToString(currency);
}

static std::string accountTypeFromAccount(const BankAccount& account) {
    if (dynamic_cast<const PersonalAccount*>(&account)) return "personal";
    if (dynamic_cast<const SavingsAccount*>(&account)) return "savings";
    if (dynamic_cast<const RetirementAccount*>(&account)) return "retirement";
    if (dynamic_cast<const InvestmentAccount*>(&account)) return "investment";
    return "personal";
}

void JsonStore::saveAll(const Bank& bank) const {
    nlohmann::json bankJson;
    bankJson["name"] = bank.getName();
    bankJson["swiftCode"] = bank.getSwiftCode();
    writeJsonFile(bankPath, bankJson);

    nlohmann::json clientsJson = nlohmann::json::array();
    nlohmann::json accountsJson = nlohmann::json::array();
    nlohmann::json bondsJson = nlohmann::json::array();
    nlohmann::json loansJson = nlohmann::json::array();

    for (const auto& clientPtr : bank.getClients()) {
        const Client& client = *clientPtr;
        clientsJson.push_back({
            {"cnp", client.getCNP()},
            {"name", client.getName()},
            {"address", client.getAddress()},
            {"monthlyIncome", client.getMonthlyIncome()}
        });

        for (const auto& accountPtr : client.getAccounts()) {
            const BankAccount& account = *accountPtr;
            nlohmann::json acc{
                {"ownerCnp", client.getCNP()},
                {"type", accountTypeFromAccount(account)},
                {"iban", account.getIBAN()},
                {"balance", account.getBalance()},
                {"currency", currencyToCode(account.getCurrency())},
                {"inceptionDate", account.getInceptionDate()}
            };

            if (auto* retirement = dynamic_cast<const RetirementAccount*>(&account)) {
                acc["maturityDate"] = retirement->getMaturityDate();
            }
            if (auto* investment = dynamic_cast<const InvestmentAccount*>(&account)) {
                for (const auto& bond : investment->getBonds()) {
                    bondsJson.push_back({
                        {"ownerIban", account.getIBAN()},
                        {"id", bond.id},
                        {"faceValue", bond.faceValue},
                        {"annualCouponRate", bond.annualCouponRate},
                        {"issueDate", bond.issueDate},
                        {"maturityDate", bond.maturityDate},
                        {"currency", currencyToCode(bond.currency)},
                        {"lastCouponYear", bond.lastCouponYear}
                    });
                }
            }

            accountsJson.push_back(acc);
        }

        for (const auto& loan : client.getLoans()) {
            loansJson.push_back({
                {"cnp", client.getCNP()},
                {"id", loan.id},
                {"principal", loan.principal},
                {"annualInterestRate", loan.annualInterestRate},
                {"termMonths", loan.termMonths},
                {"startDate", loan.startDate},
                {"nextDueDate", loan.nextDueDate},
                {"paymentIBAN", loan.paymentIBAN},
                {"currency", currencyToCode(loan.currency)},
                {"monthlyPayment", loan.monthlyPayment},
                {"remainingBalance", loan.remainingBalance},
                {"status", loan.status == PAID ? "PAID" : loan.status == OVERDUE ? "OVERDUE" : "ACTIVE"},
                {"missedPayments", loan.missedPayments}
            });
        }
    }

    writeJsonFile(clientsPath, clientsJson);
    writeJsonFile(accountsPath, accountsJson);
    writeJsonFile(bondsPath, bondsJson);
    writeJsonFile(loansPath, loansJson);
}
