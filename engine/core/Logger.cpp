#include "core/Logger.h"

#include <windows.h>

namespace Engine {

void Logger::SetConsoleColor(LogLevel level) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    switch (level) {
        case LogLevel::Trace: SetConsoleTextAttribute(hConsole, 8);   break; // Dark gray
        case LogLevel::Info:  SetConsoleTextAttribute(hConsole, 10);  break; // Green
        case LogLevel::Warn:  SetConsoleTextAttribute(hConsole, 14);  break; // Yellow
        case LogLevel::Error: SetConsoleTextAttribute(hConsole, 12);  break; // Red
        case LogLevel::Fatal: SetConsoleTextAttribute(hConsole, 79);  break; // White on red
        default: break;
    }
}

void Logger::ResetConsoleColor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 7); // Default white
}

} // namespace Engine
