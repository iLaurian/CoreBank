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

    static double calculateMonthlyPayment(double amount, double annualRate, int months);
    bool isClientRegistered(const Client* client) const;

    Bank(const std::string& name, const std::string& swiftCode);

public:
    static void initialize(const std::string& name, const std::string& swiftCode);
    static Bank& instance();
    ~Bank();

    Bank(const Bank&) = delete;
    Bank& operator=(const Bank&) = delete;
    Bank(Bank&&) noexcept = default;
    Bank& operator=(Bank&&) noexcept = default;

    const std::string& getName() const;
    const std::string& getSwiftCode() const;

    Client* registerClient(const std::string& cnp, const std::string& clientName, const std::string& clientAddress, double monthlyIncome);
    void removeClient(const std::string& cnp);
    Client* getClient(const std::string& cnp) const;

    BankAccount* findAccountByIBAN(const std::string& cnp) const;

    void processTransfer(const std::string& fromIBAN, const std::string& toIBAN, double amount, const std::string& dateStr);

    LoanRequestResult evaluateLoanRequest(const Client& client, double amount, int months) const;
    void applyMonthlyLoanPayments(const std::string& dateStr);

    double calculateTotalBankAssets() const;

    friend std::ostream& operator<<(std::ostream& os, const Bank& bank);
};

#endif
