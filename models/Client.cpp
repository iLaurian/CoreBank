#include "Client.h"

#include <cmath>

Client::Client(const std::string &cnp, const std::string &name, const std::string &address, double monthlyIncome)
    : cnp(cnp), name(name), address(address), monthlyIncome(monthlyIncome) {
}

Client::~Client() {
}

void Client::updateCreditScore() {
    int baseScore = 300;

    int incomeFactor = std::min(300, static_cast<int>(monthlyIncome / 25));

    double netWorth = calculateTotalNetWorth();
    int netWorthFactor = std::min(200, static_cast<int>(netWorth / 50));

    int validExternalTransactions = 0;
    for (const BankAccount* acc : accounts) {
        for (int i = 0; i < acc->getTransactionCount(); ++i) {
            const Transaction& t = acc->getTransaction(i);

            if (t.getType() == "TRANSFER") {
                std::string otherIBAN = (t.getSourceIBAN() == acc->getIBAN()) ? t.getTargetIBAN() : t.getSourceIBAN();

                bool isInternalTransfer = false;
                for (const BankAccount* innerAcc : accounts) {
                    if (innerAcc->getIBAN() == otherIBAN) {
                        isInternalTransfer = true;
                        break;
                    }
                }

                if (!isInternalTransfer) {
                    validExternalTransactions++;
                }
            } else {
                validExternalTransactions++;
            }
        }
    }


    int activityFactor = std::min(50, validExternalTransactions * 2);

    creditScore = baseScore + incomeFactor + netWorthFactor + activityFactor;
}

const std::string & Client::getCNP() const {
    return cnp;
}

const std::string & Client::getName() const {
    return name;
}

const std::string & Client::getAddress() const {
    return address;
}

double Client::getMonthlyIncome() const {
    return monthlyIncome;
}

int Client::getCreditScore() const {
    return creditScore;
}

void Client::addBankAccount(BankAccount *account) {
    if (!account) return;

    for (const BankAccount *acc : accounts) {
        if (acc->getIBAN() == account->getIBAN()) {
            return;
        }
    }

    accounts.push_back(account);
    updateCreditScore();
}

void Client::removeBankAccount(const std::string &iban) {
    for (auto it = accounts.begin(); it != accounts.end(); ++it) {
        if ((*it)->getIBAN() == iban) {
            delete *it;
            accounts.erase(it);
            updateCreditScore();
            return;
        }
    }
    throw std::invalid_argument("Account with IBAN " + iban + " not found.");
}

BankAccount* Client::getBankAccount(const std::string &iban) const {
    for (const auto account : accounts) {
        if (account->getIBAN() == iban) {
            return account;
        }
    }
    throw std::invalid_argument("Account with IBAN " + iban + " not found.");
}

double Client::calculateTotalNetWorth() const {
    double total = 0;
    for (const BankAccount *acc : accounts) {
        total += acc->getBalance();
    }
    return total;
}

void Client::evaluateLoanEligibility(double loanAmount, int months) const {
    if (creditScore < 500) {
        std::cout << "Loan denied: Credit score too low.\n";
        return;
    }

    double annualInterestRate = 0.12 - ((creditScore - 500) * 0.0002);
    if (annualInterestRate < 0.04) annualInterestRate = 0.04;

    const double monthlyInterestRate = annualInterestRate / 12;
    const double mathPower = std::pow(1 + monthlyInterestRate, months);
    const double monthlyPayment = loanAmount * (monthlyInterestRate * mathPower) / (mathPower - 1);

    const double maxAllowedPayment = monthlyIncome * 0.4;
    if (monthlyPayment > maxAllowedPayment) {
        std::cout << "Loan rejected: Monthly payment exceeds 40% of monthly income.\n";
        return;
    }

    std::cout << "\n--- Loan Evaluation ---\n";
    std::cout << "Status: APPROVED.\n";
    std::cout << "Requested amount: " << loanAmount << " for " << months << " months.\n";
    std::cout << "Applied interest rate: " << (annualInterestRate * 100) << "% annually.\n";
    std::cout << "Estimated monthly payment: " << monthlyPayment << "\n";
}


void Client::transferBetweenOwnAccounts(const std::string &fromIBAN, const std::string &toIBAN, double amount, const std::string& dateStr) {
    if (fromIBAN == toIBAN) {
        throw std::invalid_argument("Source and target IBAN cannot be the same.");
    }

    BankAccount* source = getBankAccount(fromIBAN);
    BankAccount* target = getBankAccount(toIBAN);

    source->processOutgoingTransfer(amount, fromIBAN, dateStr);
    target->processIncomingTransfer(amount, toIBAN, dateStr);

    updateCreditScore();
}

std::ostream & operator<<(std::ostream &os, const Client &client) {
    os << "Client(Name: " << client.name << ", CNP: " << client.cnp
       << ", Address: " << client.address << ", Monthly Income: " << client.monthlyIncome
       << ", Credit Score: " << client.creditScore << ")\n";

    os << "Bank Accounts:\n";
    for (const BankAccount* acc : client.accounts) {
        os << *acc << "\n";
    }

    return os;
}
