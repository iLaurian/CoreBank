#include "BankService.h"
#include <typeinfo>
#include <sstream>
#include "../models/cbaccount/PersonalAccount.h"
#include "../models/cbaccount/SavingsAccount.h"
#include "../models/cbaccount/RetirementAccount.h"
#include "../models/cbaccount/InvestmentAccount.h"
#include "../utils/cbvalidation/Validator.h"
#include "../utils/cbexception/BankExceptions.h"
#include "../utils/cblogger/Logger.h"

BankService::BankService(Bank& bankRef,
    std::string bankFile,
    std::string clientsFile,
    std::string accountsFile,
    std::string bondsFile,
    std::string loansFile)
    : bank(bankRef),
      ibanCounter(1),
      store(std::move(bankFile), std::move(clientsFile), std::move(accountsFile), std::move(bondsFile), std::move(loansFile)) {
    const std::string prefix = "RO12CBIN";
    long long maxSuffix = 0;
    for (const auto& client : bank.getClients()) {
        for (const auto& acc : client->getAccounts()) {
            const std::string& iban = acc->getIBAN();
            if (iban.size() > prefix.size() && iban.substr(0, prefix.size()) == prefix) {
                long long suffix = std::stoll(iban.substr(prefix.size()));
                if (suffix > maxSuffix) {
                    maxSuffix = suffix;
                }
            }
        }
    }
    ibanCounter = maxSuffix + 1;
}

std::mutex& BankService::getMutex() {
    return mutex;
}

void BankService::persist() const {
    store.saveAll(bank);
}

Client* BankService::getClient(const std::string& cnp) {
    return bank.getClient(cnp);
}

const std::vector<std::unique_ptr<Client>>& BankService::getClients() const {
    return bank.getClients();
}

Client* BankService::registerClient(const std::string& cnp, const std::string& name, const std::string& address, double monthlyIncome) {
    return bank.registerClient(cnp, name, address, monthlyIncome);
}

void BankService::removeClient(const std::string& cnp) {
    bank.removeClient(cnp);
}

BankAccount* BankService::findAccount(const std::string& iban) {
    return bank.findAccountByIBAN(iban);
}

BankAccount* BankService::findAccount(const std::string& iban, AccountKind kind) {
    {
        std::ostringstream oss;
        oss << &bank;
        Logger::info("BankService Bank instance address: " + oss.str() + " clients=" + std::to_string(bank.getClients().size()));
    }
    BankAccount* account = bank.findAccountByIBAN(iban);
    if (!account) return nullptr;
    switch (kind) {
        case AccountKind::Personal:
            if (dynamic_cast<PersonalAccount*>(account)) return account;
            break;
        case AccountKind::Savings:
            if (dynamic_cast<SavingsAccount*>(account)) return account;
            break;
        case AccountKind::Retirement:
            if (dynamic_cast<RetirementAccount*>(account)) return account;
            break;
        case AccountKind::Investment:
            if (dynamic_cast<InvestmentAccount*>(account)) return account;
            break;
        default:
            return nullptr;
    }
    const std::string requested = accountKindToString(kind);
    const std::string actual = typeid(*account).name();
    Logger::warn("Account type mismatch for IBAN: " + iban + " requested=" + requested + " actual=" + actual);
    return nullptr;
}

Client* BankService::findClientByAccountIBAN(const std::string& iban) {
    return bank.findClientByAccountIBAN(iban);
}

std::string BankService::generateIban() {
    std::string suffix = std::to_string(ibanCounter++);
    while (suffix.size() < 12) {
        suffix.insert(suffix.begin(), '0');
    }
    return std::string("RO12CBIN") + suffix;
}

BankAccount* BankService::createAccount(const std::string& cnp,
    AccountKind kind,
    double initialBalance,
    Currency currency,
    const std::string& inceptionDate,
    const std::string& maturityDate) {
    Client* client = bank.getClient(cnp);
    if (!client) {
        throw NotFoundError("Client not found.");
    }
    const std::string iban = generateIban();
    std::unique_ptr<BankAccount> account;
    switch (kind) {
        case AccountKind::Personal:
            account = std::make_unique<PersonalAccount>(iban, initialBalance, currency, inceptionDate);
            break;
        case AccountKind::Savings:
            account = std::make_unique<SavingsAccount>(iban, initialBalance, currency, inceptionDate);
            break;
        case AccountKind::Retirement:
            if (maturityDate.empty()) {
                throw ValidationError("maturityDate required for retirement account");
            }
            account = std::make_unique<RetirementAccount>(iban, initialBalance, currency, maturityDate, inceptionDate);
            break;
        case AccountKind::Investment:
            account = std::make_unique<InvestmentAccount>(iban, initialBalance, currency, inceptionDate);
            break;
        default:
            throw ValidationError("Unknown account type");
    }

    client->addBankAccount(std::move(account));
    return client->getBankAccount(iban);
}

bool BankService::removeAccount(const std::string& iban) {
    Client* client = bank.findClientByAccountIBAN(iban);
    if (!client) return false;
    return client->removeBankAccount(iban);
}

bool BankService::processTransfer(const std::string& fromIban, const std::string& toIban, double amount, const std::string& dateStr) {
    return bank.processTransfer(fromIban, toIban, amount, dateStr);
}
