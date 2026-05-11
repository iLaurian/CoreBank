#ifndef OOP_DATEUTILS_H
#define OOP_DATEUTILS_H

#include <string>

struct DateParts {
    int year;
    int month;
    int day;
};

DateParts parseDate(const std::string& dateStr);
std::string getCurrentDate();
bool isValidDate(const std::string& dateStr);
int daysBetween(const std::string& fromDate, const std::string& toDate);
std::string addMonths(const std::string& dateStr, int months);

#endif
