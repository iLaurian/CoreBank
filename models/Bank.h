#ifndef OOP_BANK_H
#define OOP_BANK_H

#include <string>
#include <vector>
#include <iostream>
#include "Client.h"

class Bank {
    std::string name;
    std::string swiftCode;
    std::vector<Client*> clients;

public:
    Bank(const std::string& name, const std::string& swiftCode);
    ~Bank();

    const std::string& getName() const;
    const std::string& getSwiftCode() const;

    void addClient(Client* client);
    void removeClient(const std::string& cnp);
    Client* getClient(const std::string& cnp) const;

    BankAccount* findAccountByIBAN(const std::string& cnp) const;

    void processTransfer(const std::string& fromIBAN, const std::string& toIBAN, double amount, const std::string& dateStr);

    double calculateTotalBankAssets() const;

    friend std::ostream& operator<<(std::ostream& os, const Bank& bank);
};

#endif