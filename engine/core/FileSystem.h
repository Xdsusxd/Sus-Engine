#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>

namespace Engine {

class FileSystem {
public:
    // ── Read operations ───────────────────────────────────────
    static std::optional<std::string> ReadText(const std::string& path);
    static std::optional<std::vector<char>> ReadBinary(const std::string& path);

    // ── Write operations ──────────────────────────────────────
    static bool WriteText(const std::string& path, const std::string& content);
    static bool WriteBinary(const std::string& path, const std::vector<char>& data);

    // ── Path utilities ────────────────────────────────────────
    static bool Exists(const std::string& path);
    static bool IsFile(const std::string& path);
    static bool IsDirectory(const std::string& path);
    static bool CreateDir(const std::string& path);

    static std::string GetExtension(const std::string& path);
    static std::string GetFileName(const std::string& path);
    static std::string GetParentPath(const std::string& path);

    // Returns the directory where the executable is located
    static std::string GetExecutablePath();

    // Resolve a path relative to the executable directory
    static std::string ResolvePath(const std::string& relativePath);
};

} // namespace Engine
