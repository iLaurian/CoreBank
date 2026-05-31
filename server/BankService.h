#ifndef OOP_BANKSERVICE_H
#define OOP_BANKSERVICE_H

#include <mutex>
#include <string>
#include "../models/cbbank/Bank.h"
#include "../utils/cbjson/JsonStore.h"
#include "ApiTypes.h"

class BankService {
    Bank& bank;
    std::mutex mutex;
    long long ibanCounter;
    JsonStore store;

    std::string generateIban();

public:
    explicit BankService(Bank& bankRef,
        std::string bankFile,
        std::string clientsFile,
        std::string accountsFile,
        std::string bondsFile,
        std::string loansFile);

    std::mutex& getMutex();
    void persist() const;

    Client* getClient(const std::string& cnp);
    const std::vector<std::unique_ptr<Client>>& getClients() const;
    Client* registerClient(const std::string& cnp, const std::string& name, const std::string& address, double monthlyIncome);
    void removeClient(const std::string& cnp);

    BankAccount* findAccount(const std::string& iban);
    BankAccount* findAccount(const std::string& iban, AccountKind kind);
    Client* findClientByAccountIBAN(const std::string& iban);

    BankAccount* createAccount(const std::string& cnp,
        AccountKind kind,
        double initialBalance,
        Currency currency,
        const std::string& inceptionDate,
        const std::string& maturityDate);

    bool removeAccount(const std::string& iban);

    bool processTransfer(const std::string& fromIban, const std::string& toIban, double amount, const std::string& dateStr);
};

#endif
