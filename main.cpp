#include <iostream>
#include <exception>
#include "models/Bank.h"
#include "models/Client.h"
#include "models/BankAccount.h"

int main() {
    try {
        Bank myBank("CoreBank International", "CBINTRO");

        std::cout << myBank.getSwiftCode() << "\n";

        Client* client1 = new Client("1900101123456", "John Doe", "123 Silicon Valley", 4500.00);
        Client* client2 = new Client("2900202234567", "Jane Smith", "456 Business Blvd", 8000.00);

        Client* dummyClient = new Client("0000000000000", "Dummy", "Nowhere", 1000.00);

        BankAccount* acc1_usd = new BankAccount("RO12CBIN000000000001", 1500.00, USD);
        BankAccount* acc2_eur = new BankAccount("RO12CBIN000000000002", 500.00, EUR);
        BankAccount* acc3_gbp = new BankAccount("RO12CBIN000000000003", 2000.00, GBP);

        BankAccount* acc_dummy = new BankAccount("RO12CBIN999999999999", 100.00, USD);

        client1->addBankAccount(acc1_usd);
        client1->addBankAccount(acc2_eur);
        client1->addBankAccount(acc_dummy);
        client2->addBankAccount(acc3_gbp);

        myBank.addClient(client1);
        myBank.addClient(client2);
        myBank.addClient(dummyClient);

        std::cout << client1->getName() << "\n";
        std::cout << client1->getAddress() << "\n";
        std::cout << client1->getMonthlyIncome() << "\n";
        std::cout << client1->getCreditScore() << "\n";

        const Client* retrievedClient = myBank.getClient("1900101123456");
        std::cout << retrievedClient->getName() << "\n";

        acc1_usd->processDeposit(300.00, "2026-03-22");
        acc1_usd->processWithdrawal(50.00, "2026-03-22");

        client1->transferBetweenOwnAccounts(
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

        client1->evaluateLoanEligibility(15000.00, 60);

        client1->removeBankAccount("RO12CBIN999999999999");
        myBank.removeClient("0000000000000");

        std::cout << myBank;

    } catch (const std::exception& e) {
        std::cerr << "EXCEPTION CAUGHT: " << e.what() << "\n";
    }

    return 0;
}