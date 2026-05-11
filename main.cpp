#include <iostream>
#include <exception>
#include <memory>
#include "models/Bank.h"
#include "models/Client.h"
#include "models/BankAccount.h"
#include "models/PersonalAccount.h"
#include "models/SavingsAccount.h"
#include "models/RetirementAccount.h"

int main() {
    try {
        Bank::initialize("CoreBank International", "CBINTRO");
        Bank& myBank = Bank::instance();

        std::cout << myBank.getSwiftCode() << "\n";

        Client* client1 = myBank.registerClient("1900101123456", "John Doe", "123 Silicon Valley", 4500.00);
        Client* client2 = myBank.registerClient("2900202234567", "Jane Smith", "456 Business Blvd", 8000.00);
        myBank.registerClient("0000000000000", "Dummy", "Nowhere", 1000.00);

        auto acc1_usd = std::make_unique<PersonalAccount>("RO12CBIN000000000001", 1500.00, USD);
        auto acc2_eur = std::make_unique<SavingsAccount>("RO12CBIN000000000002", 500.00, EUR);
        auto acc3_gbp = std::make_unique<RetirementAccount>("RO12CBIN000000000003", 2000.00, GBP, "2030-01-01");
        auto acc_dummy = std::make_unique<SavingsAccount>("RO12CBIN999999999999", 100.00, USD);

        client1->addBankAccount(std::move(acc1_usd));
        client1->addBankAccount(std::move(acc2_eur));
        client1->addBankAccount(std::move(acc_dummy));
        client2->addBankAccount(std::move(acc3_gbp));

        Client* retrievedClient = myBank.getClient("1900101123456");
        std::cout << retrievedClient->getName() << "\n";
        std::cout << retrievedClient->getAddress() << "\n";
        std::cout << retrievedClient->getMonthlyIncome() << "\n";
        std::cout << retrievedClient->getCreditScore() << "\n";

        BankAccount* acc1_usd_ptr = retrievedClient->getBankAccount("RO12CBIN000000000001");
        if (acc1_usd_ptr) {
            acc1_usd_ptr->processDeposit(300.00, "2026-03-22");
            acc1_usd_ptr->processWithdrawal(50.00, "2026-03-22");

            auto clonedAccount = acc1_usd_ptr->clone();
            std::cout << *clonedAccount;
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

        myBank.applyMonthlyAccountFees("2026-04-01");

        retrievedClient->applyInterestIfDue("2026-04-21");

        retrievedClient->closeSavingsAccount("RO12CBIN999999999999", "RO12CBIN000000000001", "2026-04-01");

        retrievedClient->removeBankAccount("RO12CBIN000000000002");

        const auto loanResult = retrievedClient->requestLoan(5000.00, 24, "2026-04-21", "RO12CBIN000000000001");
        if (loanResult.approved) {
            myBank.applyMonthlyLoanPayments("2026-05-21");
        } else {
            std::cout << "Loan request denied: " << loanResult.reason << "\n";
        }

        myBank.removeClient("0000000000000");

        std::cout << myBank;

    } catch (const std::exception& e) {
        std::cerr << "EXCEPTION CAUGHT: " << e.what() << "\n";
    }

    return 0;
}
