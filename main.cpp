#include <iostream>
#include "Transaction.h"

int main() {
    try {
        Transaction t1("DEPOSIT", "USD", "2024-01-15", 99.99, "ATM", "RO12INGB34567890");
        std::cout << t1 << '\n';

        Transaction t2("TRANSFER", "EUR", "2024-01-16", 500.00, "RO12INGB34567890", "RO99BTRL12345678");
        std::cout << t2 << '\n';

    } catch (const std::exception& e) {
        std::cerr << "Error creating transaction: " << e.what() << '\n';
    }

    return 0;
}