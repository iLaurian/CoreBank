#include "Bank.h"
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include "../utils/CurrencyConverter.h"
#include "../utils/DateUtils.h"

namespace {
    std::string g_bankName;
    std::string g_bankSwift;
    bool g_bankInitialized = false;
}

Bank::Bank(const std::string &name, const std::string &swiftCode)
    : name(name), swiftCode(swiftCode) {
}

void Bank::initialize(const std::string& bankName, const std::string& bankSwiftCode) {
    g_bankName = bankName;
    g_bankSwift = bankSwiftCode;
    g_bankInitialized = true;
}

Bank& Bank::instance() {
    if (!g_bankInitialized) {
        throw std::runtime_error("Bank::initialize must be called before Bank::instance().");
    }
    static Bank instance(g_bankName, g_bankSwift);
    return instance;
}

Bank::~Bank() {
    clients.clear();
}

const std::string& Bank::getName() const {
    return name;
}

const std::string& Bank::getSwiftCode() const {
    return swiftCode;
}

Client* Bank::registerClient(const std::string& cnp, const std::string& clientName, const std::string& clientAddress, double monthlyIncome) {
    for (const auto &c : clients) {
        if (c->getCNP() == cnp) {
            throw std::invalid_argument("Client with this CNP already exists in the bank.");
        }
    }

    auto client = std::make_unique<Client>(cnp, clientName, clientAddress, monthlyIncome);
    Client* stored = client.get();
    clients.push_back(std::move(client));
    return stored;
}

void Bank::removeClient(const std::string &cnp) {
    const auto it = std::ranges::find_if(clients, [&](const std::unique_ptr<Client> &client) {
                return client->getCNP() == cnp;
    });

    if (it == clients.end()) {
        throw std::invalid_argument("Client with CNP " + cnp + " not found.");
    }

    clients.erase(it);
}

Client* Bank::getClient(const std::string &cnp) const {
    for (const auto &client : clients) {
        if (client->getCNP() == cnp) {
            return client.get();
        }
    }
    throw std::invalid_argument("Client not found.");
}

BankAccount *Bank::findAccountByIBAN(const std::string &iban) const {
    for (const auto &client : clients) {
        if (BankAccount* acc = client->getBankAccount(iban); acc != nullptr) {
            return acc;
        }
    }
    return nullptr;
}

void Bank::processTransfer(const std::string &fromIBAN, const std::string &toIBAN, double amount, const std::string &dateStr) {
    if (fromIBAN == toIBAN) {
        throw std::invalid_argument("Source and target IBAN cannot be the same.");
    }

    BankAccount *sourceAccount = findAccountByIBAN(fromIBAN);
    if (!sourceAccount) {
        return;
    }
    sourceAccount->processOutgoingTransfer(amount, toIBAN, dateStr);

    BankAccount *targetAccount = findAccountByIBAN(toIBAN);
    if (targetAccount) {
        double convertedAmount = CurrencyConverter::convert(amount, sourceAccount->getCurrency(), targetAccount->getCurrency());
        targetAccount->processIncomingTransfer(convertedAmount, fromIBAN, dateStr);
    }
}

double Bank::calculateMonthlyPayment(double amount, double annualRate, int months) {
    const double monthlyInterestRate = annualRate / 12.0;
    const double mathPower = std::pow(1 + monthlyInterestRate, months);
    return amount * (monthlyInterestRate * mathPower) / (mathPower - 1);
}

bool Bank::isClientRegistered(const Client* client) const {
    if (!client) {
        return false;
    }
    for (const auto &existing : clients) {
        if (existing.get() == client) {
            return true;
        }
    }
    return false;
}

LoanRequestResult Bank::evaluateLoanRequest(const Client& client, double amount, int months) const {
    if (!isClientRegistered(&client)) {
        return {false, "Client not registered with this bank.", std::nullopt};
    }
    if (amount <= 0 || months <= 0) {
        return {false, "Invalid loan parameters.", std::nullopt};
    }

    const int score = client.getCreditScore();
    if (score < 500) {
        return {false, "Credit score too low.", std::nullopt};
    }

    double annualInterestRate = 0.12 - ((score - 500) * 0.0002);
    if (annualInterestRate < 0.04) annualInterestRate = 0.04;

    const double monthlyPayment = calculateMonthlyPayment(amount, annualInterestRate, months);
    const double maxAllowedPayment = client.getMonthlyIncome() * 0.4;
    if (monthlyPayment > maxAllowedPayment) {
        return {false, "Monthly payment exceeds 40% of monthly income.", std::nullopt};
    }

    Loan loan{};
    loan.id = Loan::generateId();
    loan.principal = amount;
    loan.annualInterestRate = annualInterestRate;
    loan.termMonths = months;
    loan.monthlyPayment = monthlyPayment;
    loan.remainingBalance = amount;
    loan.status = ACTIVE;
    loan.missedPayments = 0;

    return {true, "Approved.", loan};
}

void Bank::applyMonthlyLoanPayments(const std::string& dateStr) {
    for (const auto &client : clients) {
        for (auto &loan : client->getLoans()) {
            if (loan.status == PAID) {
                continue;
            }
            if (daysBetween(loan.nextDueDate, dateStr) < 0) {
                continue;
            }

            BankAccount* source = client->getBankAccount(loan.paymentIBAN);
            if (!source) {
                loan.status = OVERDUE;
                loan.missedPayments++;
                loan.nextDueDate = addMonths(loan.nextDueDate, 1);
                continue;
            }

            const double payment = loan.monthlyPayment;
            if (!source->tryPay(payment, dateStr)) {
                loan.status = OVERDUE;
                loan.missedPayments++;
                loan.nextDueDate = addMonths(loan.nextDueDate, 1);
                continue;
            }

            loan.remainingBalance -= payment;
            if (loan.remainingBalance <= 0.0) {
                loan.remainingBalance = 0.0;
                loan.status = PAID;
            } else {
                loan.status = ACTIVE;
            }

            loan.nextDueDate = addMonths(loan.nextDueDate, 1);
        }
    }
}

double Bank::calculateTotalBankAssets() const {
    double totalBankAssets = 0.0;
    for (const auto &client : clients) {
        totalBankAssets += client->calculateTotalNetWorth();
    }
    return totalBankAssets;
}

std::ostream& operator<<(std::ostream& os, const Bank& bank) {
    os << "BANK: " << bank.name << " | SWIFT: " << bank.swiftCode << "\n"
       << "TOTAL ASSETS: " << bank.calculateTotalBankAssets() << " USD\n"
       << "TOTAL CLIENTS: " << bank.clients.size() << "\n";

    for (const auto &client : bank.clients) {
        os << *client << "\n";
    }

    return os;
}
