#ifndef OOP_PERSONALACCOUNT_H
#define OOP_PERSONALACCOUNT_H

#include "BankAccount.h"

class PersonalAccount : public BankAccount {
public:
    PersonalAccount(const std::string& iban, double initialBalance, Currency curr = USD, const std::string& inceptionDate = "");
    std::unique_ptr<BankAccount> clone() const override;

protected:
    void printDetails(std::ostream& os) const override;
    double calculateMonthlyFee() const override;
};

#endif
