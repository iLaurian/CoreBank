#include "AccountFactory.h"

#include "../../utils/cbvalidation/Validator.h"
#include "../../utils/cbexception/BankExceptions.h"
#include "PersonalAccount.h"
#include "SavingsAccount.h"
#include "RetirementAccount.h"
#include "InvestmentAccount.h"
#include "../../utils/cbcurrency/CurrencyConverter.h"

std::unique_ptr<BankAccount> AccountFactory::createAccount(const nlohmann::json& item) {
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

    const Currency currency = CurrencyConverter::parseCurrency(currencyCode);

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
