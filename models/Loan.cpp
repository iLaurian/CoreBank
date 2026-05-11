#include "Loan.h"

std::string Loan::generateId() {
    static int counter = 1000;
    return "LN" + std::to_string(++counter);
}
