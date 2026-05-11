#ifndef OOP_CLIENT_H
#define OOP_CLIENT_H

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include "BankAccount.h"
#include "Loan.h"

class Bank;

class Client {
    std::string cnp;
    std::string name;
    std::string address;
    double monthlyIncome;
    std::vector<std::unique_ptr<BankAccount>> accounts;
    std::vector<Loan> loans;

    int calculateCreditScore() const;

public:
    Client(const std::string& cnp, const std::string& name, const std::string& address, double monthlyIncome);
    ~Client();

    Client(const Client& other);
    Client& operator=(Client other);
    Client(Client&&) noexcept = default;
    Client& operator=(Client&&) noexcept = default;

    const std::string& getCNP() const;
    const std::string& getName() const;
    const std::string& getAddress() const;
    double getMonthlyIncome() const;
    int getCreditScore() const;

    void addBankAccount(std::unique_ptr<BankAccount> account);
    bool removeBankAccount(const std::string& iban);
    BankAccount* getBankAccount(const std::string& iban) const;

    double calculateTotalNetWorth() const;
    void transferBetweenOwnAccounts(const std::string& fromIBAN, const std::string& toIBAN, double amount, const std::string& dateStr);
    bool closeSavingsAccount(const std::string& fromIBAN, const std::string& toIBAN, const std::string& dateStr);
    void applyInterestIfDue(const std::string& dateStr);
    void applyMonthlyAccountFees(const std::string& dateStr);
    LoanRequestResult requestLoan(double amount, int months, const std::string& dateStr, const std::string& targetIBAN);

    std::vector<Loan>& getLoans();
    const std::vector<Loan>& getLoans() const;

    std::vector<std::unique_ptr<BankAccount>>& getAccounts();
    const std::vector<std::unique_ptr<BankAccount>>& getAccounts() const;

    friend void swap(Client& first, Client& second) noexcept;
    friend std::ostream& operator<< (std::ostream& os, const Client& client);
};

#endif
