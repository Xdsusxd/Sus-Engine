#include "core/Logger.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <iostream>
#endif

namespace Engine {

void Logger::SetConsoleColor(LogLevel level) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    switch (level) {
        case LogLevel::Trace: SetConsoleTextAttribute(hConsole, 8);   break; // Dark gray
        case LogLevel::Info:  SetConsoleTextAttribute(hConsole, 10);  break; // Green
        case LogLevel::Warn:  SetConsoleTextAttribute(hConsole, 14);  break; // Yellow
        case LogLevel::Error: SetConsoleTextAttribute(hConsole, 12);  break; // Red
        case LogLevel::Fatal: SetConsoleTextAttribute(hConsole, 79);  break; // White on red
        default: break;
    }
#else
    switch (level) {
        case LogLevel::Trace: std::cout << "\033[90m"; break; // Dark gray
        case LogLevel::Info:  std::cout << "\033[32m"; break; // Green
        case LogLevel::Warn:  std::cout << "\033[33m"; break; // Yellow
        case LogLevel::Error: std::cout << "\033[31m"; break; // Red
        case LogLevel::Fatal: std::cout << "\033[41;37m"; break; // White on red
        default: break;
    }
#endif
}

void Logger::ResetConsoleColor() {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 7); // Default white
#else
    std::cout << "\033[0m"; // Reset
#endif
}

} // namespace Engine
