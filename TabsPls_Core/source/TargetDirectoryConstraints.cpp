#include <TabsPlsCore/TargetDirectoryConstraints.hpp>

#include <algorithm>

#include <TabsPlsCore/FileSystem.hpp>

namespace TargetDirectoryConstraints {

bool DirIsRoot(const FileSystem::RawPath& dir) { return dir == FileSystem::_getRootPath(dir); }

FileSystem::RawPath RemoveWhitespace(FileSystem::RawPath string) {
    string.erase(std::remove_if(string.begin(), string.end(), [](auto element) { return std::isspace(element); }),
                 string.end());
    return string;
}

bool IsIncompleteWindowsRootPath(FileSystem::RawPath path) {
#if defined(WIN32) || defined(_WIN32)
    path = RemoveWhitespace(path);
    return DirIsRoot(path) && path.back() == L':';
#else // When not on Windows, it can't be considered an incomplete Windows root path
    return false;
#endif
}
} // namespace TargetDirectoryConstraints