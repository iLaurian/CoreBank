#include "Bank.h"
#include <algorithm>
#include <stdexcept>
#include "../utils/CurrencyConverter.h"

Bank::Bank(const std::string &name, const std::string &swiftCode)
    : name(name), swiftCode(swiftCode) {
}

Bank::~Bank() {
    for (Client* client : clients) {
        delete client;
    }
    clients.clear();
}

const std::string& Bank::getName() const {
    return name;
}

const std::string& Bank::getSwiftCode() const {
    return swiftCode;
}

void Bank::addClient(Client *client) {
    if (!client) return;

    for (const Client *c : clients) {
        if (c->getCNP() == client->getCNP()) {
            throw std::invalid_argument("Client with this CNP already exists in the bank.");
        }
    }

    clients.push_back(client);
}

void Bank::removeClient(const std::string &cnp) {
    const auto it = std::ranges::find_if(clients, [&](const Client *client) {
                return client->getCNP() == cnp;
    });

    if (it == clients.end()) {
        throw std::invalid_argument("Client with CNP " + cnp + " not found.");
    }

    delete *it;
    clients.erase(it);
}

Client* Bank::getClient(const std::string &cnp) const {
    for (Client* client : clients) {
        if (client->getCNP() == cnp) {
            return client;
        }
    }
    throw std::invalid_argument("Client not found.");
}

BankAccount *Bank::findAccountByIBAN(const std::string &iban) const {
    for (const Client* client : clients) {
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

double Bank::calculateTotalBankAssets() const {
    double totalBankAssets = 0.0;
    for (const Client* client : clients) {
        totalBankAssets += client->calculateTotalNetWorth();
    }
    return totalBankAssets;
}

std::ostream& operator<<(std::ostream& os, const Bank& bank) {
    os << "BANK: " << bank.name << " | SWIFT: " << bank.swiftCode << "\n"
       << "TOTAL ASSETS: " << bank.calculateTotalBankAssets() << " USD\n"
       << "TOTAL CLIENTS: " << bank.clients.size() << "\n";

    for (const Client* client : bank.clients) {
        os << *client << "\n";
    }

    return os;
}