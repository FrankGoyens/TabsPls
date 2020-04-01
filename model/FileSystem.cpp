#include "FileSystem.hpp"

#include <filesystem>

#include <FileSystemDirectory.hpp>

namespace FileSystem
{

    bool IsDirectory(const RawPath& dir)
    {
        return std::filesystem::is_directory(dir);
    }

    RawPathVector GetFilesInCurrentDirectory()
    {
        return _getFilesInDirectory(std::filesystem::current_path().string());
    }

    RawPathVector GetFilesInDirectory(const Directory& dir)
    {
        return _getFilesInDirectory(dir.path());
    }

    RawPathVector _getFilesInDirectory(const RawPath& dir)
    {
        RawPathVector files;
        for(auto& it: std::filesystem::directory_iterator(dir))
            files.push_back(it.path().string());

        return files;
    }
}