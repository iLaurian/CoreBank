#include "Bank.h"
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include "../../utils/cbcurrency/CurrencyConverter.h"
#include "../../utils/cbdate/DateUtils.h"
#include "../../utils/cblogger/Logger.h"
#include "../../utils/cbexception/BankExceptions.h"
#include "../../utils/cbbanking/ReportGenerator.h"
#include "../cbaccount/InvestmentAccount.h"

std::unique_ptr<Bank> Bank::instancePtr = nullptr;

Bank::Bank(const std::string &name, const std::string &swiftCode)
    : name(name), swiftCode(swiftCode) {
}

void Bank::initialize(const std::string& bankName, const std::string& bankSwiftCode) {
    if (instancePtr) {
        Logger::error("Bank already initialized");
        throw ConfigurationError("Bank::initialize called more than once.");
    }
    instancePtr = std::unique_ptr<Bank>(new Bank(bankName, bankSwiftCode));
    Logger::info("Bank initialized: " + bankName + " (" + bankSwiftCode + ")");
}

Bank& Bank::instance() {
    if (!instancePtr) {
        Logger::error("Bank instance requested before initialization");
        throw ConfigurationError("Bank::initialize must be called before Bank::instance().");
    }
    return *instancePtr;
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
            Logger::error("Client registration failed: duplicate CNP " + cnp);
            throw ValidationError("Client with this CNP already exists in the bank.");
        }
    }

    auto client = std::make_unique<Client>(cnp, clientName, clientAddress, monthlyIncome);
    Client* stored = client.get();
    clients.push_back(std::move(client));
    Logger::info("Client registered: " + cnp + " " + clientName);
    return stored;
}

void Bank::removeClient(const std::string &cnp) {
    const auto it = std::ranges::find_if(clients, [&](const std::unique_ptr<Client> &client) {
                return client->getCNP() == cnp;
    });

    if (it == clients.end()) {
        Logger::error("Client removal failed: CNP not found " + cnp);
        throw NotFoundError("Client with CNP " + cnp + " not found.");
    }

    clients.erase(it);
    Logger::info("Client removed: " + cnp);
}

Client* Bank::getClient(const std::string &cnp) const {
    for (const auto &client : clients) {
        if (client->getCNP() == cnp) {
            return client.get();
        }
    }
    Logger::error("Client lookup failed: CNP not found " + cnp);
    throw NotFoundError("Client not found.");
}

BankAccount *Bank::findAccountByIBAN(const std::string &iban) const {
    for (const auto &client : clients) {
        if (BankAccount* acc = client->getBankAccount(iban); acc != nullptr) {
            return acc;
        }
    }
    Logger::warn("Account not found in bank for IBAN: " + iban);
    return nullptr;
}

void Bank::processTransfer(const std::string &fromIBAN, const std::string &toIBAN, double amount, const std::string &dateStr) {
    if (fromIBAN == toIBAN) {
        Logger::error("Transfer failed: same IBAN " + fromIBAN);
        return;
    }

    BankAccount *sourceAccount = findAccountByIBAN(fromIBAN);
    if (!sourceAccount) {
        Logger::error("Transfer failed: source account not found " + fromIBAN);
        return;
    }
    if (!sourceAccount->processOutgoingTransfer(amount, toIBAN, dateStr)) {
        return;
    }

    BankAccount *targetAccount = findAccountByIBAN(toIBAN);
    if (targetAccount) {
        double convertedAmount = CurrencyConverter::convert(amount, sourceAccount->getCurrency(), targetAccount->getCurrency());
        targetAccount->processIncomingTransfer(convertedAmount, fromIBAN, dateStr);
        Logger::info("Transfer completed: " + fromIBAN + " -> " + toIBAN);
    } else {
        Logger::warn("Transfer target account not found (external): " + toIBAN);
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
        Logger::error("Loan evaluation failed: client not registered");
        return {false, "Client not registered with this bank.", std::nullopt};
    }
    if (amount <= 0 || months <= 0) {
        Logger::error("Loan evaluation failed: invalid parameters");
        return {false, "Invalid loan parameters.", std::nullopt};
    }

    const int score = client.getCreditScore();
    if (score < 500) {
        Logger::warn("Loan evaluation denied: credit score too low");
        return {false, "Credit score too low.", std::nullopt};
    }

    double annualInterestRate = 0.12 - ((score - 500) * 0.0002);
    if (annualInterestRate < 0.04) annualInterestRate = 0.04;

    const double monthlyPayment = calculateMonthlyPayment(amount, annualInterestRate, months);
    const double maxAllowedPayment = client.getMonthlyIncome() * 0.4;
    if (monthlyPayment > maxAllowedPayment) {
        Logger::warn("Loan evaluation denied: payment exceeds income threshold");
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

    Logger::info("Loan evaluation approved for amount " + std::to_string(amount));

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
                Logger::warn("Loan payment missed: source account not found for loan " + loan.id);
                continue;
            }

            const double payment = loan.monthlyPayment;
            if (!source->tryPay(payment, dateStr)) {
                loan.status = OVERDUE;
                loan.missedPayments++;
                loan.nextDueDate = addMonths(loan.nextDueDate, 1);
                Logger::warn("Loan payment missed: insufficient funds for loan " + loan.id);
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
    Logger::info("Monthly loan payments processed for date: " + dateStr);
}

void Bank::applyMonthlyAccountFees(const std::string& dateStr) {
    for (const auto &client : clients) {
        client->applyMonthlyAccountFees(dateStr);
    }
    Logger::info("Monthly account fees applied for date: " + dateStr);
}

void Bank::applyAnnualBondCoupons(const std::string& dateStr) {
    const DateParts current = parseDate(dateStr);
    for (const auto &client : clients) {
        for (auto &account : client->getAccounts()) {
            auto* investment = dynamic_cast<InvestmentAccount*>(account.get());
            if (!investment) {
                continue;
            }

            auto& bonds = investment->getBonds();
            for (size_t i = 0; i < bonds.size(); ) {
                Bond& bond = bonds[i];
                const DateParts maturity = parseDate(bond.maturityDate);
                const DateParts issue = parseDate(bond.issueDate);

                const bool matured = daysBetween(bond.maturityDate, dateStr) >= 0;
                const bool couponAnniversary = current.month == maturity.month && current.day == maturity.day;
                const bool inRange = daysBetween(bond.issueDate, dateStr) >= 0 && daysBetween(dateStr, bond.maturityDate) >= 0;
                const bool afterIssueAnniversary = (current.year > issue.year) ||
                    (current.year == issue.year &&
                     (current.month > issue.month || (current.month == issue.month && current.day >= issue.day)));
                const bool couponDue = couponAnniversary && inRange && afterIssueAnniversary && current.year > bond.lastCouponYear;

                if (couponDue) {
                    const double coupon = bond.annualCoupon();
                    investment->creditInterest(coupon, dateStr);
                    bond.lastCouponYear = current.year;
                    Logger::info("Annual bond coupon paid: " + bond.id + " amount=" + std::to_string(coupon));
                }

                if (matured) {
                    double payout = bond.faceValue;
                    if (!couponDue && couponAnniversary && current.year > bond.lastCouponYear) {
                        payout += bond.annualCoupon();
                    }
                    if (payout > bond.faceValue) {
                        investment->creditInterest(payout - bond.faceValue, dateStr);
                    }
                    investment->creditPrincipal(bond.faceValue, dateStr);
                    Logger::info("Bond matured: " + bond.id + " payout=" + std::to_string(payout));
                    bonds.erase(bonds.begin() + static_cast<long long>(i));
                    continue;
                }

                ++i;
            }
        }
    }
    Logger::info("Annual bond coupons processed for date: " + dateStr);
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

    ReportGenerator rpt("CLIENT PORTFOLIO");
    os << rpt.generateTable(bank.clients,
        {"Name", "CNP", "Income", "Net Worth", "Score"},
        [](const std::unique_ptr<Client>& c, size_t col) -> std::string {
            switch (col) {
                case 0: return c->getName();
                case 1: return c->getCNP();
                case 2: return formatCurrency(static_cast<int>(c->getMonthlyIncome()), USD);
                case 3: return formatCurrency(c->calculateTotalNetWorth(), USD);
                case 4: return std::to_string(c->getCreditScore());
                default: return "";
            }
        });

    return os;
}
