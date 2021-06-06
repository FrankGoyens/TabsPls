#pragma once

#include <ctime>
#include <optional>
#include <variant>

#include <FileSystemDefs.hpp>

namespace FileSystem {
class Directory;
namespace Algorithm {
RawPath StripTrailingPathSeparators(RawPath);
RawPath StripLeadingPathSeparators(RawPath);
RawPath CombineDirectoryAndName(const FileSystem::Directory&, const FileSystem::RawPath&);

std::string FormatAsFileTimestamp(const std::time_t&);

struct ScaledFileSize {
    std::variant<int, float> value = 0.f;
    const char* unit;

    //! \brief Floating point comparison is done with Epsilon
    bool operator==(const FileSystem::Algorithm::ScaledFileSize&) const;
};

ScaledFileSize ScaleSizeToLargestPossibleUnit(uintmax_t bytes);
std::string Format(ScaledFileSize, std::optional<int> mantissaLength = {});

} // namespace Algorithm
} // namespace FileSystem
