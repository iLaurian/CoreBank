#ifndef OOP_RETIREMENTACCOUNT_H
#define OOP_RETIREMENTACCOUNT_H

#include "BankAccount.h"

class RetirementAccount : public BankAccount {
public:
    static constexpr double EarlyWithdrawalPenaltyRate = 0.05;

    RetirementAccount(const std::string& iban, double initialBalance, Currency curr, const std::string& maturityDate, const std::string& inceptionDate = "");
    std::unique_ptr<BankAccount> clone() const override;
    void processWithdrawal(double amount, const std::string& dateStr) override;
    void processOutgoingTransfer(double amount, const std::string& toIBAN, const std::string& dateStr) override;
    void applyInterestIfDue(const std::string& dateStr);

protected:
    void printDetails(std::ostream& os) const override;

private:
    std::string maturityDate;
    std::string lastInterestDate;
    static constexpr double AnnualInterestRate = 0.07;
    bool isMaturityReached(const std::string& dateStr) const;
    double calculateAccruedInterest(const std::string& upToDate) const;
};

#endif
