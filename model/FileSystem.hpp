#pragma once

#include <vector>
#include <string>

namespace FileSystem
{
    class Directory;
    class FilePath;

    using Filename = std::string;
    using Directoryname = std::string;

    using RawPath = std::string;
    using RawPathVector = std::vector<RawPath>;

    bool IsDirectory(const RawPath& path);
    bool IsRegularFile(const RawPath& path);

    RawPath RemoveFilename(const FilePath&);
    Filename GetFilename(const FilePath&);
    Directoryname GetDirectoryname(const Directory&);

    RawPath GetWorkingDirectory();

    RawPathVector GetFilesInCurrentDirectory();
    
    RawPathVector GetFilesInDirectory(const Directory& dir);
    RawPathVector _getFilesInDirectory(const RawPath& dir);

    RawPath GetParent(const Directory& dir);
}