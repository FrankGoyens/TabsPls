#include "FileSystemDirectory.hpp"

namespace FileSystem
{
    std::optional<Directory> Directory::FromPath(const FileSystem::RawPath& path)
    {
        if (!IsDirectory(path))
            return {};

        return Directory(path);
    }
}