#ifndef OOP_INTERESTACCRUAL_H
#define OOP_INTERESTACCRUAL_H

#include <string>
#include <vector>
#include "../models/Transaction.h"

struct InterestAccrual {
    static double calculate(const std::vector<Transaction>& transactions,
                            const std::string& iban,
                            const std::string& fromDate,
                            const std::string& toDate,
                            double annualRate,
                            double currentBalance);
};

#endif
