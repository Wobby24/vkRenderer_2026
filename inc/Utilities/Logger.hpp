#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>
#include <sstream>

/*class Logger {
public:
    enum class Severity { INFO, WARNING, ERROR };

    Logger(const std::string& filename) : log_file(filename, std::ios::app) {}

    void log(Severity severity, const std::string& message) {
        std::string color_code;
        switch (severity) {
        case Severity::INFO: color_code = "\033[34m"; break;    // Blue
        case Severity::WARNING: color_code = "\033[33m"; break; // Yellow
        case Severity::ERROR: color_code = "\033[31m"; break;   // Red
        }

        std::string timestamp = get_current_time();
        std::string severity_str = to_string(severity);

        if (log_file.is_open()) {
            log_file << "[" << timestamp << "] [" << severity_str << "] " << message << std::endl;
        }

        std::cout << color_code << "[" << timestamp << "] [" << severity_str << "] " << message << "\033[0m" << std::endl;
    }

    template<typename... Args>
    void log(Severity severity, const std::string& format, Args... args) {
        std::ostringstream oss;
        format_string(oss, format, args...);
        log(severity, oss.str());
    }

private:
    std::ofstream log_file;

    std::string to_string(Severity severity) {
        switch (severity) {
        case Severity::INFO: return "INFO";
        case Severity::WARNING: return "WARNING";
        case Severity::ERROR: return "ERROR";
        default: return "UNKNOWN";
        }
    }

    std::string get_current_time() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        char buffer[20];
        struct tm time_info;
        localtime_s(&time_info, &now_time);
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &time_info);
        return buffer;
    }

    template<typename T>
    void format_string(std::ostringstream& oss, const std::string& format, T value) {
        size_t start = format.find("{}");
        if (start != std::string::npos) {
            oss << format.substr(0, start) << value << format.substr(start + 2);
        }
        else {
            oss << format;
        }
    }

    template<typename T, typename... Args>
    void format_string(std::ostringstream& oss, const std::string& format, T value, Args... args) {
        size_t start = format.find("{}");
        if (start != std::string::npos) {
            oss << format.substr(0, start) << value;
            format_string(oss, format.substr(start + 2), args...);
        }
        else {
            oss << format;
        }
    }
};*/

namespace Aero::Utilities {
    class Logger {
    public:
        std::ofstream outFile_;
        enum class Severity { INFO, WARNING, ERROR };

        // 1. Your new global access point
        static Logger& instance() {
            static Logger instance;
            return instance;
        }

        // 2. Prevent copying and moving
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        Logger(Logger&&) = delete;
        Logger& operator=(Logger&&) = delete;

        Logger(const std::string& filename) : outFile_(filename, std::ios_base::trunc) {}
        Logger() : outFile_("LOG.txt", std::ios_base::trunc) {}
        ~Logger() = default;

        void log(Severity severity, const std::string& msg) {
            std::string color_code;
            switch (severity) {
            case Severity::INFO: color_code = "\033[34m"; break;    // Blue
            case Severity::WARNING: color_code = "\033[33m"; break; // Yellow
            case Severity::ERROR: color_code = "\033[31m"; break;   // Red
            }

            std::string timestamp = get_current_time();
            std::string severity_str = to_string(severity);

            if (outFile_.good()) {
                outFile_ << "[" << timestamp << "] [" << severity_str << "] " << msg << std::endl;
            }

            std::cout << color_code << "[" << timestamp << "] [" << severity_str << "] " << msg << "\033[0m" << std::endl;
        }
    private:

        std::string to_string(Severity severity) {
            switch (severity) {
            case Severity::INFO: return "INFO";
            case Severity::WARNING: return "WARNING";
            case Severity::ERROR: return "ERROR";
            default: return "UNKNOWN";
            }
        }

        std::string get_current_time() {
            auto now = std::chrono::system_clock::now();
            std::time_t now_time = std::chrono::system_clock::to_time_t(now);
            char buffer[20];
            struct tm time_info;
            localtime_s(&time_info, &now_time);
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &time_info);
            return buffer;
        }
    };
}