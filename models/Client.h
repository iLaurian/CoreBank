#ifndef OOP_CLIENT_H
#define OOP_CLIENT_H

#include <string>
#include <vector>
#include <memory>
#include "BankAccount.h"

class Client {
    std::string cnp;
    std::string name;
    std::string address;
    double monthlyIncome;
    std::vector<std::unique_ptr<BankAccount>> accounts;

    int calculateCreditScore() const;

public:
    Client(const std::string& cnp, const std::string& name, const std::string& address, double monthlyIncome);
    ~Client();

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    Client(Client&&) noexcept = default;
    Client& operator=(Client&&) noexcept = default;

    const std::string& getCNP() const;
    const std::string& getName() const;
    const std::string& getAddress() const;
    double getMonthlyIncome() const;
    int getCreditScore() const;

    void addBankAccount(std::unique_ptr<BankAccount> account);
    void removeBankAccount(const std::string& iban);
    BankAccount* getBankAccount(const std::string& iban) const;

    double calculateTotalNetWorth() const;
    void evaluateLoanEligibility(double loanAmount, int months) const;
    void transferBetweenOwnAccounts(const std::string& fromIBAN, const std::string& toIBAN, double amount, const std::string& dateStr);
    void closeSavingsAccount(const std::string& fromIBAN, const std::string& toIBAN, const std::string& dateStr);
    void applyInterestIfDue(const std::string& dateStr);

    friend std::ostream& operator<< (std::ostream& os, const Client& client);
};

#endif
