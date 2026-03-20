#include <iostream>
#include "models/Transaction.h"
#include "models/BankAccount.h"

int main() {
    try {
        BankAccount myAccount("RO12INGB34567890", 1000.00);

        std::cout << myAccount.getIBAN() << "\n";

        myAccount.processDeposit(500.0, "USD", "2024-03-01");
        myAccount.processWithdrawal(200.0, "USD", "2024-03-02");
        myAccount.processDeposit(150.0, "USD", "2024-03-03");
        myAccount.processWithdrawal(50.0, "USD", "2024-03-04");
        myAccount.processDeposit(300.0, "USD", "2024-03-05");
        myAccount.processDeposit(100.0, "USD", "2024-03-06");

        Transaction manualTx("PAYMENT", "USD", "2024-03-10", 75.00, myAccount.getIBAN(), "RO99UTILITATI123");
        myAccount.addTransaction(manualTx);

        std::cout << myAccount.getTransactionCount() << "\n\n";

        std::cout << myAccount << "\n\n";

        BankAccount savingsAccount = myAccount;

        savingsAccount.processDeposit(9000.0, "USD", "2024-03-07");

        std::cout << myAccount.getBalance() << " USD\n";
        std::cout << savingsAccount.getBalance() << " USD\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }

    return 0;
}