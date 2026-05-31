#ifndef OOP_LOGGER_H
#define OOP_LOGGER_H

#include <string>
#include <fstream>

class Logger {
public:
    static void init(const std::string& directory = "./logs");
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);

private:
    static void log(const std::string& level, const std::string& message);
    static std::string currentTimestamp();
    static std::string fileTimestamp();
    static std::ofstream& stream();
};

#endif
