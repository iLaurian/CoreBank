#ifndef OOP_SAVINGSACCOUNT_H
#define OOP_SAVINGSACCOUNT_H

#include "BankAccount.h"

class SavingsAccount : public BankAccount {
public:
    static constexpr double MinimumBalance = 100.0;

    SavingsAccount(const std::string& iban, double initialBalance, Currency curr = USD, const std::string& inceptionDate = "");
    std::unique_ptr<BankAccount> clone() const override;
    void processWithdrawal(double amount, const std::string& dateStr) override;
    void processOutgoingTransfer(double amount, const std::string& toIBAN, const std::string& dateStr) override;
    void applyInterestIfDue(const std::string& dateStr);
    void closeAndTransferTo(BankAccount& target, const std::string& dateStr);

protected:
    void printDetails(std::ostream& os) const override;
    double calculateMonthlyFee() const override;

private:
    static constexpr double AnnualInterestRate = 0.03;
    std::string lastInterestDate;
    double calculateAccruedInterest(const std::string& upToDate) const;
};

#endif
