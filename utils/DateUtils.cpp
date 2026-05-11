#include "DateUtils.h"
#include <stdexcept>
#include "Logger.h"
#include "BankExceptions.h"
#include <ctime>

static bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int daysInMonth(int year, int month) {
    static const int daysByMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month == 2) {
        return isLeapYear(year) ? 29 : 28;
    }
    return daysByMonth[month - 1];
}

static bool isDigitChar(char c) {
    return c >= '0' && c <= '9';
}

DateParts parseDate(const std::string& dateStr) {
    if (dateStr.size() != 10 || dateStr[4] != '-' || dateStr[7] != '-') {
        Logger::error("Invalid date format: " + dateStr);
        throw ValidationError("Invalid date format: " + dateStr);
    }

    for (size_t i = 0; i < dateStr.size(); ++i) {
        if (i == 4 || i == 7) {
            continue;
        }
        if (!isDigitChar(dateStr[i])) {
            Logger::error("Invalid date format: " + dateStr);
            throw ValidationError("Invalid date format: " + dateStr);
        }
    }

    DateParts parts {
        std::stoi(dateStr.substr(0, 4)),
        std::stoi(dateStr.substr(5, 2)),
        std::stoi(dateStr.substr(8, 2))
    };

    if (parts.month < 1 || parts.month > 12) {
        Logger::error("Invalid date format: " + dateStr);
        throw ValidationError("Invalid date format: " + dateStr);
    }

    const int maxDay = daysInMonth(parts.year, parts.month);
    if (parts.day < 1 || parts.day > maxDay) {
        Logger::error("Invalid date format: " + dateStr);
        throw ValidationError("Invalid date format: " + dateStr);
    }

    return parts;
}

bool isValidDate(const std::string& dateStr) {
    try {
        parseDate(dateStr);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

static int dateToSerialDays(const DateParts& parts) {
    int days = 0;
    for (int year = 0; year < parts.year; ++year) {
        days += isLeapYear(year) ? 366 : 365;
    }
    for (int month = 1; month < parts.month; ++month) {
        days += daysInMonth(parts.year, month);
    }
    days += parts.day;
    return days;
}

int daysBetween(const std::string& fromDate, const std::string& toDate) {
    const DateParts from = parseDate(fromDate);
    const DateParts to = parseDate(toDate);
    return dateToSerialDays(to) - dateToSerialDays(from);
}

std::string getCurrentDate() {
    std::time_t t = std::time(nullptr);
    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &t);
#else
    local = *std::localtime(&t);
#endif

    const int year = local.tm_year + 1900;
    const int month = local.tm_mon + 1;
    const int day = local.tm_mday;

    auto twoDigits = [](int value) {
        if (value < 10) {
            return std::string("0") + std::to_string(value);
        }
        return std::to_string(value);
    };

    return std::to_string(year) + "-" + twoDigits(month) + "-" + twoDigits(day);
}

std::string addMonths(const std::string& dateStr, int months) {
    DateParts parts = parseDate(dateStr);
    int totalMonths = (parts.year * 12) + (parts.month - 1) + months;
    if (totalMonths < 0) {
        Logger::error("Invalid month adjustment: " + dateStr);
        throw ValidationError("Invalid month adjustment: " + dateStr);
    }

    const int newYear = totalMonths / 12;
    const int newMonth = (totalMonths % 12) + 1;
    const int maxDay = daysInMonth(newYear, newMonth);
    const int newDay = parts.day > maxDay ? maxDay : parts.day;

    auto twoDigits = [](int value) {
        if (value < 10) {
            return std::string("0") + std::to_string(value);
        }
        return std::to_string(value);
    };

    return std::to_string(newYear) + "-" + twoDigits(newMonth) + "-" + twoDigits(newDay);
}
