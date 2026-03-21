#ifndef OOP_TRANSACTION_H
#define OOP_TRANSACTION_H

#include <string>
#include <iostream>

enum Currency {
    USD, EUR, GBP, JPY
};

enum TransactionType {
    DEPOSIT, WITHDRAWAL, TRANSFER, PAYMENT
};

class Transaction {
    std::string id;
    std::string date;
    double amount;
    TransactionType type;
    Currency currency;
    std::string sourceIBAN;
    std::string targetIBAN;

    static std::string generateUniqueId();
    static void validateTransactionLogic(TransactionType type, const std::string& srcIBAN, const std::string& tgtIBAN);

public:
    Transaction() : amount(0), type(DEPOSIT), currency(USD) {}
    Transaction(TransactionType tType, Currency curr,
                const std::string& dateStr, double amt,
                const std::string& srcIBAN, const std::string& tgtIBAN);

    const std::string& getId() const;
    const std::string& getDate() const;
    double getAmount() const;
    std::string getType() const;
    std::string getCurrency() const;
    const std::string& getSourceIBAN() const;
    const std::string& getTargetIBAN() const;

    friend std::ostream& operator<<(std::ostream& os, const Transaction& t);
};

#endif