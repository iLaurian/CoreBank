#include "Client.h"
#include <algorithm>
#include <cmath>
#include "Bank.h"
#include "../utils/CurrencyConverter.h"
#include "../utils/DateUtils.h"
#include "../utils/Logger.h"
#include "SavingsAccount.h"
#include "RetirementAccount.h"

Client::Client(const std::string &cnp, const std::string &name, const std::string &address, double monthlyIncome)
    : cnp(cnp), name(name), address(address), monthlyIncome(monthlyIncome) {
}

Client::Client(const Client& other)
    : cnp(other.cnp), name(other.name), address(other.address), monthlyIncome(other.monthlyIncome), loans(other.loans) {
    accounts.reserve(other.accounts.size());
    for (const auto &acc : other.accounts) {
        if (acc) {
            accounts.push_back(acc->clone());
        }
    }
}

Client& Client::operator=(Client other) {
    swap(*this, other);
    return *this;
}

Client::~Client() {
    accounts.clear();
    Logger::info("Client destroyed: " + cnp);
}

void swap(Client& first, Client& second) noexcept {
    using std::swap;
    swap(first.cnp, second.cnp);
    swap(first.name, second.name);
    swap(first.address, second.address);
    swap(first.monthlyIncome, second.monthlyIncome);
    swap(first.accounts, second.accounts);
    swap(first.loans, second.loans);
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
            Logger::warn("Duplicate account not added: " + account->getIBAN());
            return;
        }
    }

    Logger::info("Account added to client: " + cnp + " IBAN=" + account->getIBAN());
    accounts.push_back(std::move(account));
}

bool Client::removeBankAccount(const std::string &iban) {
    const auto it = std::ranges::find_if(accounts, [&](const std::unique_ptr<BankAccount> &acc) {
                return acc->getIBAN() == iban;
    });

    if (it == accounts.end()) {
        Logger::error("Remove account failed, IBAN not found: " + iban);
        return false;
    }

    accounts.erase(it);
    Logger::info("Account removed: " + iban);
    return true;
}

BankAccount* Client::getBankAccount(const std::string &iban) const {
    for (const auto &account : accounts) {
        if (account->getIBAN() == iban) {
            return account.get();
        }
    }
    Logger::warn("Account not found for IBAN: " + iban);
    return nullptr;
}

double Client::calculateTotalNetWorth() const {
    double totalUSD = 0;
    for (const auto &acc : accounts) {
        totalUSD += CurrencyConverter::convert(acc->getBalance(), acc->getCurrency(), USD);
    }
    return totalUSD;
}


void Client::transferBetweenOwnAccounts(const std::string &fromIBAN, const std::string &toIBAN, double amount, const std::string& dateStr) {
    if (fromIBAN == toIBAN) {
        Logger::error("Transfer between own accounts failed: same IBAN " + fromIBAN);
        return;
    }

    BankAccount* source = getBankAccount(fromIBAN);
    BankAccount* target = getBankAccount(toIBAN);

    if (!source || !target) {
        Logger::error("Transfer between own accounts failed: account not found");
        return;
    }

    if (!source->processOutgoingTransfer(amount, toIBAN, dateStr)) {
        return;
    }
    double convertedAmount = CurrencyConverter::convert(amount, source->getCurrency(), target->getCurrency());
    target->processIncomingTransfer(convertedAmount, fromIBAN, dateStr);
    Logger::info("Transfer between own accounts: " + fromIBAN + " -> " + toIBAN);
}

const std::vector<Loan>& Client::getLoans() const {
    return loans;
}

std::vector<Loan>& Client::getLoans() {
    return loans;
}

bool Client::closeSavingsAccount(const std::string& fromIBAN, const std::string& toIBAN, const std::string& dateStr) {
    if (fromIBAN == toIBAN) {
        Logger::error("Close savings failed: source and target IBAN match " + fromIBAN);
        return false;
    }

    BankAccount* target = getBankAccount(toIBAN);
    if (!target) {
        Logger::error("Close savings failed: target account not found " + toIBAN);
        return false;
    }

    const auto it = std::ranges::find_if(accounts, [&](const std::unique_ptr<BankAccount> &acc) {
        return acc->getIBAN() == fromIBAN;
    });

    if (it == accounts.end()) {
        Logger::error("Close savings failed: savings account not found " + fromIBAN);
        return false;
    }

    auto* savings = dynamic_cast<SavingsAccount*>(it->get());
    if (!savings) {
        Logger::error("Close savings failed: source not savings account " + fromIBAN);
        return false;
    }

    savings->closeAndTransferTo(*target, dateStr);

    accounts.erase(it);
    Logger::info("Savings account closed: " + fromIBAN + " -> " + toIBAN);
    return true;
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

void Client::applyMonthlyAccountFees(const std::string& dateStr) {
    for (const auto &account : accounts) {
        if (account) {
            account->applyMonthlyFee(dateStr);
        }
    }
    Logger::info("Client fees applied: " + cnp + " date=" + dateStr);
}

LoanRequestResult Client::requestLoan(double amount, int months, const std::string& dateStr, const std::string& targetIBAN) {
    const auto result = Bank::instance().evaluateLoanRequest(*this, amount, months);
    if (!result.approved) {
        return result;
    }

    BankAccount* target = getBankAccount(targetIBAN);
    if (!target) {
        Logger::error("Loan request failed: target account not found " + targetIBAN);
        return {false, "Target account not found.", std::nullopt};
    }

    Loan loan = *result.loan;
    loan.startDate = dateStr;
    loan.nextDueDate = addMonths(dateStr, 1);
    loan.paymentIBAN = targetIBAN;
    loan.currency = target->getCurrency();

    target->processDeposit(loan.principal, dateStr);
    loans.push_back(loan);

    Logger::info("Loan issued: " + loan.id + " IBAN=" + targetIBAN);

    return {true, "Approved.", loan};
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
