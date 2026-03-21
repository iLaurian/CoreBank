#ifndef OOP_CURRENCYCONVERTER_H
#define OOP_CURRENCYCONVERTER_H

#include "../models/Transaction.h"

namespace CurrencyConverter {
    double getExchangeRate(Currency from, Currency to);
    double convert(double amount, Currency from, Currency to);
}

#endif