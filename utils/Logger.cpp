#include "Logger.h"
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace {
    std::ofstream g_logStream;
}

void Logger::init(const std::string& directory) {
    std::filesystem::create_directories(directory);
    const std::string fileName = "log-" + fileTimestamp() + ".txt";
    const std::filesystem::path logPath = std::filesystem::path(directory) / fileName;
    g_logStream.open(logPath.string(), std::ios::out | std::ios::app);
    if (g_logStream.is_open()) {
        log("INFO", "Logger initialized at " + logPath.string());
    }
}

void Logger::info(const std::string& message) {
    log("INFO", message);
}

void Logger::warn(const std::string& message) {
    log("WARN", message);
}

void Logger::error(const std::string& message) {
    log("ERROR", message);
}

void Logger::log(const std::string& level, const std::string& message) {
    if (!g_logStream.is_open()) {
        return;
    }
    g_logStream << "[" << currentTimestamp() << "] " << level << " " << message << "\n";
    g_logStream.flush();
}

std::string Logger::currentTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif
    std::ostringstream ss;
    ss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string Logger::fileTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif
    std::ostringstream ss;
    ss << std::put_time(&localTime, "%Y-%m-%d-%H%M%S");
    return ss.str();
}
