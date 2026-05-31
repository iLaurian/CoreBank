#ifndef OOP_REPORTGENERATOR_H
#define OOP_REPORTGENERATOR_H

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "../../models/cbtransaction/Transaction.h"

template<typename T>
std::string formatCurrency(T amount, Currency curr) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    switch (curr) {
        case USD: oss << "$"; break;
        case EUR: oss << "\u20AC"; break;
        case GBP: oss << "\u00A3"; break;
        case JPY: oss << "\u00A5"; break;
        case CHF: oss << "CHF "; break;
    }
    oss << amount;
    return oss.str();
}

class ReportGenerator {
    std::string title;

public:
    explicit ReportGenerator(std::string t) : title(std::move(t)) {}

    template<typename U, typename Extractor>
    std::string generateTable(
        const std::vector<U>& items,
        const std::vector<std::string>& headers,
        Extractor cellExtractor
    ) const {
        std::ostringstream oss;
        oss << title << "\n";

        std::vector<size_t> widths;
        for (const auto& h : headers) {
            widths.push_back(h.size());
        }

        for (const auto& item : items) {
            for (size_t i = 0; i < headers.size(); ++i) {
                const std::string cell = cellExtractor(item, i);
                if (cell.size() > widths[i]) {
                    widths[i] = cell.size();
                }
            }
        }

        auto separator = [&]() {
            for (auto w : widths) {
                oss << "+-" << std::string(w, '-') << "-";
            }
            oss << "+\n";
        };

        separator();
        for (size_t i = 0; i < headers.size(); ++i) {
            oss << "| " << std::left << std::setw(static_cast<int>(widths[i])) << headers[i] << " ";
        }
        oss << "|\n";
        separator();

        for (const auto& item : items) {
            for (size_t i = 0; i < headers.size(); ++i) {
                oss << "| " << std::left << std::setw(static_cast<int>(widths[i])) << cellExtractor(item, i) << " ";
            }
            oss << "|\n";
        }
        separator();

        return oss.str();
    }
};

#endif
