#ifndef OOP_INVESTMENTACCOUNT_H
#define OOP_INVESTMENTACCOUNT_H

#include "BankAccount.h"
#include "../cbinstrument/Bond.h"
#include <vector>

class InvestmentAccount : public BankAccount {
public:
    InvestmentAccount(const std::string& iban, double initialBalance, Currency curr = USD, const std::string& inceptionDate = "");
    std::unique_ptr<BankAccount> clone() const override;

    bool addBond(const Bond& bond, const std::string& dateStr);
    void creditInterest(double amount, const std::string& dateStr);
    void creditPrincipal(double amount, const std::string& dateStr);
    std::vector<Bond>& getBonds();
    const std::vector<Bond>& getBonds() const;

protected:
    void printDetails(std::ostream& os) const override;
    double calculateMonthlyFee() const override;

private:
    std::vector<Bond> bonds;
    double getNetAssetValue() const;
};

#endif
