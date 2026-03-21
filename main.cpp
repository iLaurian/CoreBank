#include <iostream>
#include <exception>
#include "models/Transaction.h"
#include "models/BankAccount.h"
#include "models/Client.h"

int main() {
    try {
        Client myClient("1900101123456", "John Doe", "123 Silicon Valley, Tech City", 4500.00);

        std::cout << myClient.getCNP() << "\n";
        std::cout << myClient.getName() << "\n";
        std::cout << myClient.getAddress() << "\n";
        std::cout << myClient.getMonthlyIncome() << "\n";
        std::cout << myClient.getCreditScore() << "\n";

        BankAccount* usdAccount = new BankAccount("RO12INGB000000000001", 1500.00, USD);
        BankAccount* eurAccount = new BankAccount("RO99BTRL000000000002", 500.00, EUR);

        myClient.addBankAccount(usdAccount);
        myClient.addBankAccount(eurAccount);

        BankAccount* accUSD = myClient.getBankAccount("RO12INGB000000000001");
        accUSD->processDeposit(300.00, "2026-03-21");
        accUSD->processWithdrawal(50.00, "2026-03-22");

        myClient.transferBetweenOwnAccounts(
            "RO12INGB000000000001",
            "RO99BTRL000000000002",
            200.00,
            "2026-03-23"
        );

        myClient.sendMoneyExternal(
            "RO99BTRL000000000002",
            "DE56SPRK999999999999",
            150.00,
            "2026-03-24"
        );

        myClient.evaluateLoanEligibility(15000.00, 60);

        myClient.removeBankAccount("RO12INGB000000000001");

        // myClient.sendMoneyExternal(
        //     "RO12INGB000000000001",
        //     "INVALID_IBAN!!!",
        //     100.00,
        //     "2026-03-25"
        // );

    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }

    return 0;
}