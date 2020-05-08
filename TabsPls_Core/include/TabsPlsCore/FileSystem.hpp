#pragma once

#include <vector>

#include <FileSystemDefs.hpp>

namespace FileSystem
{
    class Directory;
    class FilePath;

    bool IsDirectory(const RawPath& path);
    bool IsRegularFile(const RawPath& path);

    RawPath RemoveFilename(const FilePath&);
    Name GetFilename(const FilePath&);
    Name GetDirectoryname(const Directory&);
    
    Name _getRootPath(const RawPath&);
    Name _getRootName(const RawPath&);

    RawPath GetWorkingDirectory();

    RawPathVector GetFilesInCurrentDirectory();
    
    RawPathVector GetFilesInDirectory(const Directory& dir);
    RawPathVector _getFilesInDirectory(const RawPath& dir);

    RawPath GetParent(const Directory& dir);
}