#ifndef OOP_BANKACCOUNT_H
#define OOP_BANKACCOUNT_H

#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include "Transaction.h"

class BankAccount {
protected:
    std::string IBAN;
    double balance;
    Currency currency;
    std::string inceptionDate;
    std::vector<Transaction> transactions;
    int transactionCount;
    int transactionCapacity;

    void resizeTransactions();
    virtual void printDetails(std::ostream& os) const;
    virtual double calculateMonthlyFee() const = 0;

public:
    BankAccount(const std::string& iban, double initialBalance, Currency curr = USD, const std::string& inceptionDate = "");
    BankAccount(const BankAccount& other);
    BankAccount& operator=(const BankAccount& other);
    virtual ~BankAccount();
    virtual std::unique_ptr<BankAccount> clone() const = 0;

    const std::string& getIBAN() const;
    double getBalance() const;
    int getTransactionCount() const;
    Currency getCurrency() const;
    const Transaction& getTransaction(int index) const;
    bool tryPay(double amount, const std::string& dateStr);
    const std::string& getInceptionDate() const;

    virtual void addTransaction(const Transaction& t);
    virtual void processDeposit(double amount, const std::string& dateStr);
    virtual void processWithdrawal(double amount, const std::string& dateStr);
    virtual void processOutgoingTransfer(double amount, const std::string& toIBAN, const std::string& dateStr);
    virtual void processIncomingTransfer(double amount, const std::string& fromIBAN, const std::string& dateStr);
    void applyMonthlyFee(const std::string& dateStr);

    friend std::ostream& operator<<(std::ostream& os, const BankAccount& account);

};

#endif
