#ifndef OOP_CURRENCYCONVERTER_H
#define OOP_CURRENCYCONVERTER_H

#include "../../models/cbtransaction/Transaction.h"

namespace CurrencyConverter {
    Currency parseCurrency(const std::string& code);
    double getExchangeRate(Currency from, Currency to);
    double convert(double amount, Currency from, Currency to);
    std::string formatCurrency(double amount, Currency curr);

    template<typename T>
    std::string formatCurrency(T amount, Currency curr) {
        return formatCurrency(static_cast<double>(amount), curr);
    }
}

#endif
