#include "PersonalAccount.h"

PersonalAccount::PersonalAccount(const std::string& iban, double initialBalance, Currency curr, const std::string& inceptionDate)
    : BankAccount(iban, initialBalance, curr, inceptionDate) {
}

std::unique_ptr<BankAccount> PersonalAccount::clone() const {
    return std::make_unique<PersonalAccount>(*this);
}

void PersonalAccount::printDetails(std::ostream& os) const {
    os << "PersonalAccount";
}

double PersonalAccount::calculateMonthlyFee() const {
    return 5.0;
}
