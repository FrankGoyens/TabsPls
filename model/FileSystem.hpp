#pragma once

#include <vector>
#include <string>

namespace FileSystem
{
    bool IsDirectory(const std::string& dir);

    std::vector<std::string> GetFilesInCurrentDirectory();
    std::vector<std::string> GetFilesInDirectory(const std::string& dir);
}