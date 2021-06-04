#pragma once

#include <ctime>

#include <FileSystemDefs.hpp>

namespace FileSystem {
class Directory;
namespace Algorithm {
RawPath StripTrailingPathSeparators(RawPath);
RawPath StripLeadingPathSeparators(RawPath);
RawPath CombineDirectoryAndName(const FileSystem::Directory&, const FileSystem::RawPath&);

std::string FormatAsFileTimestamp(const std::time_t&);

} // namespace Algorithm
} // namespace FileSystem
