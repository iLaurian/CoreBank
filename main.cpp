#include <iostream>
#include <exception>
#include <memory>
#include "models/Bank.h"
#include "models/Client.h"
#include "models/BankAccount.h"

int main() {
    try {
        Bank myBank("CoreBank International", "CBINTRO");

        std::cout << myBank.getSwiftCode() << "\n";

        auto client1 = std::make_unique<Client>("1900101123456", "John Doe", "123 Silicon Valley", 4500.00);
        auto client2 = std::make_unique<Client>("2900202234567", "Jane Smith", "456 Business Blvd", 8000.00);

        auto dummyClient = std::make_unique<Client>("0000000000000", "Dummy", "Nowhere", 1000.00);

        auto acc1_usd = std::make_unique<BankAccount>("RO12CBIN000000000001", 1500.00, USD);
        auto acc2_eur = std::make_unique<BankAccount>("RO12CBIN000000000002", 500.00, EUR);
        auto acc3_gbp = std::make_unique<BankAccount>("RO12CBIN000000000003", 2000.00, GBP);

        auto acc_dummy = std::make_unique<BankAccount>("RO12CBIN999999999999", 100.00, USD);

        client1->addBankAccount(std::move(acc1_usd));
        client1->addBankAccount(std::move(acc2_eur));
        client1->addBankAccount(std::move(acc_dummy));
        client2->addBankAccount(std::move(acc3_gbp));

        myBank.addClient(std::move(client1));
        myBank.addClient(std::move(client2));
        myBank.addClient(std::move(dummyClient));

        Client* retrievedClient = myBank.getClient("1900101123456");
        std::cout << retrievedClient->getName() << "\n";
        std::cout << retrievedClient->getAddress() << "\n";
        std::cout << retrievedClient->getMonthlyIncome() << "\n";
        std::cout << retrievedClient->getCreditScore() << "\n";

        BankAccount* acc1_usd_ptr = retrievedClient->getBankAccount("RO12CBIN000000000001");
        if (acc1_usd_ptr) {
            acc1_usd_ptr->processDeposit(300.00, "2026-03-22");
            acc1_usd_ptr->processWithdrawal(50.00, "2026-03-22");
        }

        retrievedClient->transferBetweenOwnAccounts(
            "RO12CBIN000000000001",
            "RO12CBIN000000000002",
            50.00,
            "2026-03-22"
        );

        myBank.processTransfer(
            "RO12CBIN000000000001",
            "RO12CBIN000000000002",
            200.00,
            "2026-03-22"
        );

        myBank.processTransfer(
            "RO12CBIN000000000002",
            "RO12CBIN000000000003",
            150.00,
            "2026-03-22"
        );

        myBank.processTransfer(
            "RO12CBIN000000000003",
            "DE56SPRK999999999999",
            100.00,
            "2026-03-22"
        );

        retrievedClient->evaluateLoanEligibility(15000.00, 60);

        retrievedClient->removeBankAccount("RO12CBIN999999999999");
        myBank.removeClient("0000000000000");

        std::cout << myBank;

    } catch (const std::exception& e) {
        std::cerr << "EXCEPTION CAUGHT: " << e.what() << "\n";
    }

    return 0;
}
