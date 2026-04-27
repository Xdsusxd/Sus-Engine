#include "core/FileSystem.h"
#include "core/Logger.h"
#include <fstream>
#include <sstream>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif
namespace Engine {

std::optional<std::string> FileSystem::ReadText(const std::string& path) {
    std::ifstream file(path, std::ios::in);
    if (!file.is_open()) {
        LOG_ERROR("FileSystem", "Failed to open text file: {}", path);
        return std::nullopt;
    }

    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::optional<std::vector<char>> FileSystem::ReadBinary(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        LOG_ERROR("FileSystem", "Failed to open binary file: {}", path);
        return std::nullopt;
    }

    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(static_cast<size_t>(size));
    if (!file.read(buffer.data(), size)) {
        LOG_ERROR("FileSystem", "Failed to read binary file: {}", path);
        return std::nullopt;
    }

    return buffer;
}

bool FileSystem::WriteText(const std::string& path, const std::string& content) {
    // Ensure parent directory exists
    auto parent = std::filesystem::path(path).parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }

    std::ofstream file(path, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        LOG_ERROR("FileSystem", "Failed to open file for writing: {}", path);
        return false;
    }

    file << content;
    return file.good();
}

bool FileSystem::WriteBinary(const std::string& path, const std::vector<char>& data) {
    auto parent = std::filesystem::path(path).parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }

    std::ofstream file(path, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        LOG_ERROR("FileSystem", "Failed to open binary file for writing: {}", path);
        return false;
    }

    file.write(data.data(), static_cast<std::streamsize>(data.size()));
    return file.good();
}

bool FileSystem::Exists(const std::string& path) {
    return std::filesystem::exists(path);
}

bool FileSystem::IsFile(const std::string& path) {
    return std::filesystem::is_regular_file(path);
}

bool FileSystem::IsDirectory(const std::string& path) {
    return std::filesystem::is_directory(path);
}

bool FileSystem::CreateDir(const std::string& path) {
    std::error_code ec;
    bool result = std::filesystem::create_directories(path, ec);
    if (ec) {
        LOG_ERROR("FileSystem", "Failed to create directory '{}': {}", path, ec.message());
        return false;
    }
    return result;
}

std::string FileSystem::GetExtension(const std::string& path) {
    return std::filesystem::path(path).extension().string();
}

std::string FileSystem::GetFileName(const std::string& path) {
    return std::filesystem::path(path).filename().string();
}

std::string FileSystem::GetParentPath(const std::string& path) {
    return std::filesystem::path(path).parent_path().string();
}

std::string FileSystem::GetExecutablePath() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path().string();
#else
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        return std::filesystem::path(buffer).parent_path().string();
    }
    return "."; // Fallback
#endif
}

std::string FileSystem::ResolvePath(const std::string& relativePath) {
    return (std::filesystem::path(GetExecutablePath()) / relativePath).string();
}

} // namespace Engine
