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

static std::pair<uintmax_t, int> CalcSizeInLargestPossibleUnit(uintmax_t bytes) {
    auto sizeInLargestPossibleUnit = bytes;
    int unitIndex = 0;

    while (sizeInLargestPossibleUnit >= 1024 && unitIndex < 9) {
        ++unitIndex;

        sizeInLargestPossibleUnit /= 1024;
    }
    return {sizeInLargestPossibleUnit, unitIndex};
}

static bool MantissaIsNeeded(uintmax_t bytes, double bytesInLargestPossibleUnit) {
    return bytes > 1024 && bytesInLargestPossibleUnit != bytes;
}

static float CombineSizeAndMantissa(uintmax_t bytes, double bytesInLargestPossibleUnit,
                                    uintmax_t sizeInLargestPossibleUnit, double mantissa) {
    std::ostringstream oss;
    oss << sizeInLargestPossibleUnit;
    if (MantissaIsNeeded(bytes, bytesInLargestPossibleUnit)) {
        const auto mantissaString = std::to_string(mantissa);
        if (mantissaString.size() > 1)
            oss << std::string(mantissaString.begin() + 1, mantissaString.end());
    }

    return std::stof(oss.str());
}

ScaledFileSize ScaleSizeToLargestPossibleUnit(uintmax_t bytes) {
    if (bytes == 0)
        return {0.f, "B"};

    constexpr char* sizeUnits[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

    auto [sizeInLargestPossibleUnit, unitIndex] = CalcSizeInLargestPossibleUnit(bytes);

    const auto bytesInSingleLargestUnit = std::pow(1024, unitIndex);
    const auto bytesInLargestPossibleUnit = bytesInSingleLargestUnit * sizeInLargestPossibleUnit;
    const auto mantissa = (bytes - bytesInLargestPossibleUnit) / bytesInSingleLargestUnit;

    return {CombineSizeAndMantissa(bytes, bytesInLargestPossibleUnit, sizeInLargestPossibleUnit, mantissa),
            sizeUnits[unitIndex]};
}

std::string Format(ScaledFileSize scaledSize, std::optional<int> mantissaLength) {
    std::ostringstream oss;
    if (mantissaLength)
        oss << std::setprecision(*mantissaLength + 1);
    else
        oss << std::fixed;
    oss << scaledSize.value << " " << scaledSize.unit;
    return oss.str();
}

bool ScaledFileSize::operator==(const FileSystem::Algorithm::ScaledFileSize& other) const {
    return std::abs(value - other.value) < std::numeric_limits<float>::epsilon() && strcmp(unit, other.unit) == 0;
}

} // namespace Algorithm
} // namespace FileSystem
