#include "FileSystemDirectory.hpp"

namespace FileSystem
{
    Directory Directory::FromCurrentWorkingDirectory()
    {
        return Directory(FileSystem::GetWorkingDirectory());
    }

    std::optional<Directory> Directory::FromPath(const FileSystem::RawPath& path)
    {
        if (!IsDirectory(path))
            return {};

        return Directory(path);
    }
}