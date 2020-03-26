#include "FileSystem.hpp"

#include <filesystem>

namespace FileSystem
{
    bool IsDirectory(const std::string& dir)
    {
        return std::filesystem::is_directory(dir);
    }

    std::vector<std::string> GetFilesInCurrentDirectory()
    {
        return GetFilesInDirectory(std::filesystem::current_path().string());
    }

    std::vector<std::string> GetFilesInDirectory(const std::string& dir)
    {
        std::vector<std::string> files;
        for(auto& it: std::filesystem::directory_iterator(dir))
            files.push_back(it.path().string());

        return files;
    }
}