#pragma once

#include <vector>
#include <string>

namespace FileSystem
{
    class Directory;

    using RawPath = std::string;
    using RawPathVector = std::vector<RawPath>;

    bool IsDirectory(const RawPath& path);

    RawPath GetWorkingDirectory();

    RawPathVector GetFilesInCurrentDirectory();
    
    RawPathVector GetFilesInDirectory(const Directory& dir);
    RawPathVector _getFilesInDirectory(const RawPath& dir);

    RawPath GetParent(const Directory& dir);
}