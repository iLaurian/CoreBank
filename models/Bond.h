#ifndef OOP_BOND_H
#define OOP_BOND_H

#include <string>
#include "Transaction.h"

struct Bond {
    std::string id;
    double faceValue;
    double annualCouponRate;
    std::string issueDate;
    std::string maturityDate;
    Currency currency;
    int lastCouponYear;

    double annualCoupon() const {
        return faceValue * annualCouponRate;
    }
};

#endif
