#ifndef OOP_REPORTGENERATOR_H
#define OOP_REPORTGENERATOR_H

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <functional>

template<typename Item>
class ReportGenerator {
    std::string title;
    std::vector<std::string> headers;
    std::function<std::string(const Item&, size_t)> cellExtractor;

public:
    ReportGenerator(std::string t,
        std::vector<std::string> headerList,
        std::function<std::string(const Item&, size_t)> extractor)
        : title(std::move(t)), headers(std::move(headerList)), cellExtractor(std::move(extractor)) {}

    std::string generateTable(
        const std::vector<Item>& items
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
