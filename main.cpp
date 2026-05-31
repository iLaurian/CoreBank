#include <iostream>
#include <exception>
#include "models/Bank.h"
#include "models/Client.h"
#include "models/BankAccount.h"
#include "models/InvestmentAccount.h"
#include "utils/Logger.h"
#include "utils/JsonLoader.h"

int main() {
    try {
        Logger::init();
        Logger::info("Application started");

        Bank& myBank = JsonLoader::loadBankData(
            "data/bank.json",
            "data/clients.json",
            "data/accounts.json",
            "data/bonds.json",
            "data/loans.json");

        std::cout << myBank.getSwiftCode() << "\n";

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

        if (auto* investment = dynamic_cast<InvestmentAccount*>(myBank.getClient("2900202234567")
                ->getBankAccount("RO12CBIN000000000004"))) {
            investment->creditInterest(10.0, "2026-03-22");
        }

        myBank.applyMonthlyAccountFees("2026-04-01");

        myBank.applyAnnualBondCoupons("2027-03-22");

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
        Logger::error(std::string("Unhandled exception: ") + e.what());
    }

    return 0;
}
