#ifndef OOP_BANK_H
#define OOP_BANK_H

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include "Client.h"

class Bank {
    std::string name;
    std::string swiftCode;
    std::vector<std::unique_ptr<Client>> clients;

public:
    Bank(const std::string& name, const std::string& swiftCode);
    ~Bank();

    Bank(const Bank&) = delete;
    Bank& operator=(const Bank&) = delete;
    Bank(Bank&&) noexcept = default;
    Bank& operator=(Bank&&) noexcept = default;

    const std::string& getName() const;
    const std::string& getSwiftCode() const;

    void addClient(std::unique_ptr<Client> client);
    void removeClient(const std::string& cnp);
    Client* getClient(const std::string& cnp) const;

    BankAccount* findAccountByIBAN(const std::string& cnp) const;

    void processTransfer(const std::string& fromIBAN, const std::string& toIBAN, double amount, const std::string& dateStr);

    double calculateTotalBankAssets() const;

    friend std::ostream& operator<<(std::ostream& os, const Bank& bank);
};

#endif
