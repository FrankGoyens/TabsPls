#pragma once

#include <FileSystemDefs.hpp>

namespace FileSystem {
class Directory;
namespace Algorithm {
RawPath StripTrailingPathSeparators(RawPath);
RawPath StripLeadingPathSeparators(RawPath);
RawPath CombineDirectoryAndName(const FileSystem::Directory&, const FileSystem::RawPath&);
} // namespace Algorithm
} // namespace FileSystem
