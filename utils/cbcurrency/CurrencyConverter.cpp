#include "CurrencyConverter.h"
#include "../cblogger/Logger.h"
#include "../cbexception/BankExceptions.h"

namespace CurrencyConverter {
    double getRateToUSD(Currency curr) {
        switch(curr) {
            case USD: return 1.0;
            case EUR: return 1.16;
            case GBP: return 1.33;
            case JPY: return 0.0063;
            case CHF: return 1.27;
            default:
            Logger::error("Unsupported currency conversion");
            throw ConfigurationError("Unsupported currency");
    }
}

    double getExchangeRate(Currency from, Currency to) {
        if (from == to) return 1.0;
        double valueInUSD = getRateToUSD(from);
        double targetRateUSD = getRateToUSD(to);
        return valueInUSD / targetRateUSD;
    }

    double convert(double amount, Currency from, Currency to) {
        return amount * getExchangeRate(from, to);
    }
}
