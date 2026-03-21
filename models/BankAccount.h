#ifndef OOP_BANKACCOUNT_H
#define OOP_BANKACCOUNT_H

#include <string>
#include <iostream>
#include "Transaction.h"

class BankAccount {
protected:
    std::string IBAN;
    double balance;
    Currency currency;
    Transaction* transactions{};
    int transactionCount;
    int transactionCapacity;

    void resizeTransactions();

public:
    BankAccount(const std::string& iban, double initialBalance, Currency curr = USD);
    BankAccount(const BankAccount& other);
    BankAccount& operator=(const BankAccount& other);
    virtual ~BankAccount();

    const std::string& getIBAN() const;
    double getBalance() const;
    int getTransactionCount() const;
    Currency getCurrency() const;
    const Transaction& getTransaction(int index) const;

    virtual void addTransaction(const Transaction& t);
    virtual void processDeposit(double amount, const std::string& dateStr);
    virtual void processWithdrawal(double amount, const std::string& dateStr);
    virtual void processOutgoingTransfer(double amount, const std::string& toIBAN, const std::string& dateStr);
    virtual void processIncomingTransfer(double amount, const std::string& fromIBAN, const std::string& dateStr);

    friend std::ostream& operator<<(std::ostream& os, const BankAccount& account);

};

#endif