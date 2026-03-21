#ifndef OOP_CLIENT_H
#define OOP_CLIENT_H

#include <string>
#include <vector>
#include "BankAccount.h"

class Client {
    std::string cnp;
    std::string name;
    std::string address;
    double monthlyIncome;
    int creditScore;
    std::vector<BankAccount *> accounts;

    void updateCreditScore();

public:
    Client(const std::string& cnp, const std::string& name, const std::string& address, double monthlyIncome);
    ~Client();

    const std::string& getCNP() const;
    const std::string& getName() const;
    const std::string& getAddress() const;
    double getMonthlyIncome() const;
    int getCreditScore() const;

    void addBankAccount(BankAccount* account);
    void removeBankAccount(const std::string& iban);
    BankAccount* getBankAccount(const std::string& iban) const;

    double calculateTotalNetWorth() const;
    void evaluateLoanEligibility(double loanAmount, int months) const;
    void transferBetweenOwnAccounts(const std::string& fromIBAN, const std::string& toIBAN, double amount, const std::string& dateStr);

    friend std::ostream& operator<< (std::ostream& os, const Client& client);
};

#endif