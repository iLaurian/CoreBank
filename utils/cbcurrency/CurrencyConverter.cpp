#include "CurrencyConverter.h"
#include <iomanip>
#include <sstream>
#include "../cblogger/Logger.h"
#include "../cbexception/BankExceptions.h"

namespace CurrencyConverter {
    Currency parseCurrency(const std::string& code) {
        if (code == "USD") return USD;
        if (code == "EUR") return EUR;
        if (code == "GBP") return GBP;
        if (code == "JPY") return JPY;
        if (code == "CHF") return CHF;
        Logger::error("Unsupported currency code: " + code);
        throw ValidationError("Unsupported currency code: " + code);
    }

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

    std::string formatCurrency(double amount, Currency curr) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        switch (curr) {
            case USD: oss << "$"; break;
            case EUR: oss << "EUR "; break;
            case GBP: oss << "GBP "; break;
            case JPY: oss << "JPY "; break;
            case CHF: oss << "CHF "; break;
        }
        oss << amount;
        return oss.str();
    }
}
