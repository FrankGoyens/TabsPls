#include <TabsPlsCore/FileSystemAlgorithm.hpp>

#include <cmath>
#include <cstring>
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
#if defined(WIN32) || defined(_WIN32)
    struct tm localTimeBuf {};
    ::localtime_s(&localTimeBuf, &timestamp);
    struct tm* localTime = &localTimeBuf;
#else
    struct tm* localTime = std::localtime(&timestamp);
#endif
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

static std::variant<int, float> CombineSizeAndMantissa(uintmax_t bytes, double bytesInLargestPossibleUnit,
                                                       uintmax_t sizeInLargestPossibleUnit, double mantissa) {
    std::ostringstream oss;
    oss << sizeInLargestPossibleUnit;
    if (MantissaIsNeeded(bytes, bytesInLargestPossibleUnit)) {
        const auto mantissaString = std::to_string(mantissa);
        if (mantissaString.size() > 1)
            oss << std::string(mantissaString.begin() + 1, mantissaString.end());
        return std::stof(oss.str());
    }
    return std::stoi(oss.str());
}

ScaledFileSize ScaleSizeToLargestPossibleUnit(uintmax_t bytes) {
    if (bytes == 0)
        return {0, "B"};

    const char* sizeUnits[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

    auto [sizeInLargestPossibleUnit, unitIndex] = CalcSizeInLargestPossibleUnit(bytes);

    const auto bytesInSingleLargestUnit = std::pow(1024, unitIndex);
    const auto bytesInLargestPossibleUnit = bytesInSingleLargestUnit * sizeInLargestPossibleUnit;
    const auto mantissa = (bytes - bytesInLargestPossibleUnit) / bytesInSingleLargestUnit;

    return {CombineSizeAndMantissa(bytes, bytesInLargestPossibleUnit, sizeInLargestPossibleUnit, mantissa),
            sizeUnits[unitIndex]};
}

std::string Format(ScaledFileSize scaledSize, std::optional<int> mantissaLength) {
    std::ostringstream oss;
    if (mantissaLength && std::holds_alternative<float>(scaledSize.value))
        oss << std::setprecision(*mantissaLength + 1);

    oss << std::fixed;
    std::visit([&oss](const auto& value) { oss << value; }, scaledSize.value);
    oss << " " << scaledSize.unit;
    return oss.str();
}

bool ScaledFileSize::operator==(const FileSystem::Algorithm::ScaledFileSize& other) const {
    if (value.index() != other.value.index())
        return false;

    if (strcmp(unit, other.unit) != 0)
        return false;

    if (std::holds_alternative<float>(value))
        return std::abs(std::get<float>(value) - std::get<float>(other.value)) < 10e-6;
    else
        return std::get<int>(value) == std::get<int>(other.value);
}

} // namespace Algorithm
} // namespace FileSystem
