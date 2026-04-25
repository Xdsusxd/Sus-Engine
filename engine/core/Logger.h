#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <format>
#include <chrono>
#include <iostream>
#include <filesystem>

namespace Engine {

enum class LogLevel : int {
    Trace = 0,
    Info,
    Warn,
    Error,
    Fatal
};

class Logger {
public:
    static Logger& Get() {
        static Logger instance;
        return instance;
    }

    bool Init(const std::string& logFilePath = "fyengine.log") {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        m_FilePath = logFilePath;

        // Create directories if needed
        auto parent = std::filesystem::path(logFilePath).parent_path();
        if (!parent.empty()) {
            std::filesystem::create_directories(parent);
        }

        m_File.open(logFilePath, std::ios::out | std::ios::trunc);
        if (!m_File.is_open()) {
            std::cerr << "[LOGGER] Failed to open log file: " << logFilePath << "\n";
            return false;
        }

        m_Initialized = true;
        return true;
    }

    void Shutdown() {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        if (m_File.is_open()) {
            m_File.flush();
            m_File.close();
        }
        m_Initialized = false;
    }

    void SetMinLevel(LogLevel level) { m_MinLevel = level; }

    template<typename... Args>
    void Log(LogLevel level, const std::string& category, 
             std::format_string<Args...> fmt, Args&&... args) {
        if (level < m_MinLevel) return;

        std::string message = std::format(fmt, std::forward<Args>(args)...);
        std::string formatted = FormatMessage(level, category, message);

        std::lock_guard<std::recursive_mutex> lock(m_Mutex);

        // Console output (with color codes on Windows)
        SetConsoleColor(level);
        std::cout << formatted << "\n";
        ResetConsoleColor();

        // File output
        if (m_Initialized && m_File.is_open()) {
            m_File << formatted << "\n";
            m_File.flush();
        }
    }

private:
    Logger() = default;
    ~Logger() { Shutdown(); }
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string FormatMessage(LogLevel level, const std::string& category,
                              const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::tm tm_buf{};
        localtime_s(&tm_buf, &time);

        return std::format("[{:02}:{:02}:{:02}.{:03}] [{}] [{}] {}",
            tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec,
            ms.count(), LevelToString(level), category, message);
    }

    static const char* LevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::Trace: return "TRACE";
            case LogLevel::Info:  return "INFO ";
            case LogLevel::Warn:  return "WARN ";
            case LogLevel::Error: return "ERROR";
            case LogLevel::Fatal: return "FATAL";
            default:              return "?????";
        }
    }

    void SetConsoleColor(LogLevel level);
    void ResetConsoleColor();

    std::recursive_mutex m_Mutex;
    std::ofstream m_File;
    std::string  m_FilePath;
    bool         m_Initialized = false;
    LogLevel     m_MinLevel = LogLevel::Trace;
};

// ── Logging macros ────────────────────────────────────────────
#define LOG_TRACE(cat, fmt, ...) ::Engine::Logger::Get().Log(::Engine::LogLevel::Trace, cat, fmt, ##__VA_ARGS__)
#define LOG_INFO(cat, fmt, ...)  ::Engine::Logger::Get().Log(::Engine::LogLevel::Info,  cat, fmt, ##__VA_ARGS__)
#define LOG_WARN(cat, fmt, ...)  ::Engine::Logger::Get().Log(::Engine::LogLevel::Warn,  cat, fmt, ##__VA_ARGS__)
#define LOG_ERROR(cat, fmt, ...) ::Engine::Logger::Get().Log(::Engine::LogLevel::Error, cat, fmt, ##__VA_ARGS__)
#define LOG_FATAL(cat, fmt, ...) ::Engine::Logger::Get().Log(::Engine::LogLevel::Fatal, cat, fmt, ##__VA_ARGS__)

} // namespace Engine
