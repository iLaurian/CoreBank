#include "Client.h"
#include <algorithm>
#include <cmath>
#include "../utils/CurrencyConverter.h"
#include "SavingsAccount.h"
#include "RetirementAccount.h"

Client::Client(const std::string &cnp, const std::string &name, const std::string &address, double monthlyIncome)
    : cnp(cnp), name(name), address(address), monthlyIncome(monthlyIncome) {
}

Client::~Client() {
    accounts.clear();
}

int Client::calculateCreditScore() const {
    int baseScore = 300;

    int incomeFactor = std::min(300, static_cast<int>(monthlyIncome / 25));

    double netWorth = calculateTotalNetWorth();
    int netWorthFactor = std::min(200, static_cast<int>(netWorth / 50));

    int validExternalTransactions = 0;
    for (const auto &acc : accounts) {
        for (int i = 0; i < acc->getTransactionCount(); ++i) {
            const Transaction& t = acc->getTransaction(i);

            if (t.getType() == "TRANSFER") {
                std::string otherIBAN = (t.getSourceIBAN() == acc->getIBAN()) ? t.getTargetIBAN() : t.getSourceIBAN();

                bool isInternalTransfer = false;
                for (const auto &innerAcc : accounts) {
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

    return baseScore + incomeFactor + netWorthFactor + activityFactor;
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
    return calculateCreditScore();
}

void Client::addBankAccount(std::unique_ptr<BankAccount> account) {
    if (!account) return;

    for (const auto &acc : accounts) {
        if (acc->getIBAN() == account->getIBAN()) {
            return;
        }
    }

    accounts.push_back(std::move(account));
}

void Client::removeBankAccount(const std::string &iban) {
    const auto it = std::ranges::find_if(accounts, [&](const std::unique_ptr<BankAccount> &acc) {
                return acc->getIBAN() == iban;
    });

    if (it == accounts.end()) {
        throw std::invalid_argument("Account with IBAN " + iban + " not found.");
    }

    accounts.erase(it);
}

BankAccount* Client::getBankAccount(const std::string &iban) const {
    for (const auto &account : accounts) {
        if (account->getIBAN() == iban) {
            return account.get();
        }
    }
    return nullptr;
}

double Client::calculateTotalNetWorth() const {
    double totalUSD = 0;
    for (const auto &acc : accounts) {
        totalUSD += CurrencyConverter::convert(acc->getBalance(), acc->getCurrency(), USD);
    }
    return totalUSD;
}

void Client::evaluateLoanEligibility(double loanAmount, int months) const {
    const int score = getCreditScore();
    if (score < 500) {
        std::cout << "Loan denied: Credit score too low.\n";
        return;
    }

    double annualInterestRate = 0.12 - ((score - 500) * 0.0002);
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

    if (!source || !target) {
        return;
    }

    source->processOutgoingTransfer(amount, toIBAN, dateStr);
    double convertedAmount = CurrencyConverter::convert(amount, source->getCurrency(), target->getCurrency());
    target->processIncomingTransfer(convertedAmount, fromIBAN, dateStr);
}

void Client::closeSavingsAccount(const std::string& fromIBAN, const std::string& toIBAN, const std::string& dateStr) {
    if (fromIBAN == toIBAN) {
        throw std::invalid_argument("Source and target IBAN cannot be the same.");
    }

    BankAccount* target = getBankAccount(toIBAN);
    if (!target) {
        throw std::invalid_argument("Target account not found.");
    }

    const auto it = std::ranges::find_if(accounts, [&](const std::unique_ptr<BankAccount> &acc) {
        return acc->getIBAN() == fromIBAN;
    });

    if (it == accounts.end()) {
        throw std::invalid_argument("Savings account not found.");
    }

    auto* savings = dynamic_cast<SavingsAccount*>(it->get());
    if (!savings) {
        throw std::invalid_argument("Source account is not a savings account.");
    }

    savings->closeAndTransferTo(*target, dateStr);

    accounts.erase(it);
}

void Client::applyInterestIfDue(const std::string& dateStr) {
    for (const auto &account : accounts) {
        if (auto* savings = dynamic_cast<SavingsAccount*>(account.get())) {
            savings->applyInterestIfDue(dateStr);
            continue;
        }
        if (auto* retirement = dynamic_cast<RetirementAccount*>(account.get())) {
            retirement->applyInterestIfDue(dateStr);
        }
    }
}

std::ostream & operator<<(std::ostream &os, const Client &client) {
    os << "Client(Name: " << client.name << ", CNP: " << client.cnp
       << ", Address: " << client.address << ", Monthly Income: " << client.monthlyIncome
       << ", Credit Score: " << client.getCreditScore() << ")\n";

    os << "Bank Accounts:\n";
    for (const auto &acc : client.accounts) {
        os << *acc << "\n";
    }

    return os;
}
