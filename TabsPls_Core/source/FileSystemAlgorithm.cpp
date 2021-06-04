#include <TabsPlsCore/FileSystemAlgorithm.hpp>

#include <ctime>
#include <iomanip>
#include <sstream>

#include <TabsPlsCore/FileSystem.hpp>
#include <TabsPlsCore/FileSystemDirectory.hpp>

namespace FileSystem {
namespace Algorithm {
RawPath StripTrailingPathSeparators(RawPath path) {
    const auto sep = Separator();
    while (path.substr(path.length() - sep.length(), sep.length()) == sep) {
        path = path.substr(0, path.length() - sep.length());
    }

    return path;
}

RawPath StripLeadingPathSeparators(RawPath path) {
    const auto sep = Separator();
    while (path.substr(0, sep.length()) == sep) {
        path = path.substr(sep.length(), path.length() - sep.length());
    }

    return path;
}

RawPath CombineDirectoryAndName(const FileSystem::Directory& dir, const FileSystem::RawPath& name) {
    return FileSystem::Algorithm::StripTrailingPathSeparators(dir.path()) + FileSystem::Separator() +
           FileSystem::Algorithm::StripLeadingPathSeparators(name);
}

std::string FormatAsFileTimestamp(const std::time_t& timestamp) {
    struct tm* localTime = std::localtime(&timestamp);
    std::ostringstream oss;
    oss << std::put_time(localTime, "%a %b %d %H:%M:%S %Y");
    return oss.str();
}
} // namespace Algorithm
} // namespace FileSystem
