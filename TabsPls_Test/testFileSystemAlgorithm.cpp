#include <gtest/gtest.h>

#include <cmath>

#include <TabsPlsCore/FileSystemAlgorithm.hpp>

#include "FakeFileSystem.hpp"

static void PrintTypeName(std::ostream& os, float) {
    os.precision(std::numeric_limits<float>::max_digits10);
    os << "float";
}
static void PrintTypeName(std::ostream& os, int) { os << "int"; }

static std::ostream& operator<<(std::ostream& os, const FileSystem::Algorithm::ScaledFileSize& scaledFileSize) {
    os << "ScaledFileSize object" << std::endl;
    os << "unit = " << scaledFileSize.unit << std::endl;
    os << "value (";
    std::visit(
        [&os](const auto& value) {
            PrintTypeName(os, value);
            os << ") = " << value << std::endl;
        },
        scaledFileSize.value);
    return os;
}

namespace FileSystemAlgorithmTests {
TEST(FileSystemAlgorithmTest, StripTrailingPathSeparators) {
    auto givenPath = FakeFileSystem::MergeUsingSeparator({"C", "", ""});
    EXPECT_EQ(FileSystem::Algorithm::StripTrailingPathSeparators(givenPath), "C");

    givenPath = "C";
    EXPECT_EQ(FileSystem::Algorithm::StripTrailingPathSeparators(givenPath), "C");

    givenPath = FakeFileSystem::MergeUsingSeparator({"C", "Users", "..", "Users", ".", "Jeff", "", "", ""});
    EXPECT_EQ(FileSystem::Algorithm::StripTrailingPathSeparators(givenPath),
              FakeFileSystem::MergeUsingSeparator({"C", "Users", "..", "Users", ".", "Jeff"}));
}

TEST(FileSystemAlgorithmTest, StripLeadingPathSeparators) {
    auto givenPath = FakeFileSystem::MergeUsingSeparator({"", "", "C"});
    EXPECT_EQ(FileSystem::Algorithm::StripLeadingPathSeparators(givenPath), "C");

    givenPath = "C";
    EXPECT_EQ(FileSystem::Algorithm::StripLeadingPathSeparators(givenPath), "C");

    givenPath = FakeFileSystem::MergeUsingSeparator({
        "",
        "",
        "",
        "C",
        "Users",
        "..",
        "Users",
        ".",
        "Jeff",
    });
    EXPECT_EQ(FileSystem::Algorithm::StripLeadingPathSeparators(givenPath),
              FakeFileSystem::MergeUsingSeparator({"C", "Users", "..", "Users", ".", "Jeff"}));
}

constexpr const char* arbitraryDateString = "Mon Feb 15 16:17:18 2021";

static std::time_t MakeArbitraryDate() {
    struct tm tm;
    std::istringstream iss(arbitraryDateString);
    iss >> std::get_time(&tm, "%a %b %d %H:%M:%S %Y");
    return mktime(&tm);
}

TEST(FileSystemAlgorithmTest, FormatAsFileTimestamp) {
    const auto givenTimestamp = MakeArbitraryDate();
    EXPECT_EQ(arbitraryDateString, FileSystem::Algorithm::FormatAsFileTimestamp(givenTimestamp));
}

TEST(FileSystemAlgorithmTest, ScaleSizeToLargestPossibleUnit) {
    EXPECT_EQ((FileSystem::Algorithm::ScaledFileSize{1, "B"}),
              FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(1ULL));

    EXPECT_EQ((FileSystem::Algorithm::ScaledFileSize{1, "kB"}),
              FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(1024ULL));
    EXPECT_EQ((FileSystem::Algorithm::ScaledFileSize{1, "MB"}),
              FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(1024ULL * 1024ULL));
    EXPECT_EQ((FileSystem::Algorithm::ScaledFileSize{1, "GB"}),
              FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(1024ULL * 1024ULL * 1024ULL));
    EXPECT_EQ((FileSystem::Algorithm::ScaledFileSize{1, "TB"}),
              FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(1024ULL * 1024ULL * 1024ULL * 1024ULL));
    EXPECT_EQ((FileSystem::Algorithm::ScaledFileSize{1, "PB"}),
              FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(1024ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL));
    EXPECT_EQ((FileSystem::Algorithm::ScaledFileSize{1, "EB"}),
              FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(1024ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL *
                                                                    1024ULL));

    // Zetta bytes and onward are simply too big to store in uintmax_t
    EXPECT_EQ((FileSystem::Algorithm::ScaledFileSize{0, "B"}),
              FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(1024ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL *
                                                                    1024ULL * 1024ULL));
    EXPECT_EQ((FileSystem::Algorithm::ScaledFileSize{0, "B"}),
              FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(1024ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL *
                                                                    1024ULL * 1024ULL * 1024ULL));
}

TEST(FileSystemAlgorithmTest, ScaleSizeToLargestPossibleUnitTimes5) {
    EXPECT_EQ((FileSystem::Algorithm::ScaledFileSize{5, "B"}),
              FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(5 * 1ULL));
    EXPECT_EQ((FileSystem::Algorithm::ScaledFileSize{5, "kB"}),
              FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(5 * 1024ULL));
    EXPECT_EQ((FileSystem::Algorithm::ScaledFileSize{5, "MB"}),
              FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(5 * 1024ULL * 1024ULL));
    EXPECT_EQ((FileSystem::Algorithm::ScaledFileSize{5, "GB"}),
              FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(5 * 1024ULL * 1024ULL * 1024ULL));
    EXPECT_EQ((FileSystem::Algorithm::ScaledFileSize{5, "TB"}),
              FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(5 * 1024ULL * 1024ULL * 1024ULL * 1024ULL));
    EXPECT_EQ(
        (FileSystem::Algorithm::ScaledFileSize{5, "PB"}),
        FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(5 * 1024ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL));
    EXPECT_EQ((FileSystem::Algorithm::ScaledFileSize{5, "EB"}),
              FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(5 * 1024ULL * 1024ULL * 1024ULL * 1024ULL *
                                                                    1024ULL * 1024ULL));

    // Zetta bytes and onward is simply too big to store in uintmax_t
}

//! \brief Meant to have the same mantissa for every unit by incrementing the 'power'
static uintmax_t MakeMantissa(int power, int lowerEnd) {
    return static_cast<uintmax_t>(std::pow(1024ULL, power) * lowerEnd);
}

TEST(FileSystemAlgorithmTest, ScaleSizeToLargestPossibleUnitWithMantissa) {
    EXPECT_EQ((FileSystem::Algorithm::ScaledFileSize{5.132812f, "kB"}),
              FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(5 * 1024ULL + MakeMantissa(0, 136)));
    EXPECT_EQ((FileSystem::Algorithm::ScaledFileSize{5.132812f, "MB"}),
              FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(5 * 1024ULL * 1024ULL + MakeMantissa(1, 136)));
    EXPECT_EQ(
        (FileSystem::Algorithm::ScaledFileSize{5.132812f, "GB"}),
        FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(5 * 1024ULL * 1024ULL * 1024ULL + MakeMantissa(2, 136)));
    EXPECT_EQ((FileSystem::Algorithm::ScaledFileSize{5.132812f, "TB"}),
              FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(5 * 1024ULL * 1024ULL * 1024ULL * 1024ULL +
                                                                    MakeMantissa(3, 136)));
    EXPECT_EQ((FileSystem::Algorithm::ScaledFileSize{5.132812f, "PB"}),
              FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(
                  5 * 1024ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL + MakeMantissa(4, 136)));
    EXPECT_EQ((FileSystem::Algorithm::ScaledFileSize{5.132812f, "EB"}),
              FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(
                  5 * 1024ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL + MakeMantissa(5, 136)));

    // Zetta bytes and onward is simply too big to store in uintmax_t
}

TEST(FileSystemAlgorithmTest, FormatIntScaledFileSize) {
    const FileSystem::Algorithm::ScaledFileSize givenScaledSize{5, "kB"};
    EXPECT_EQ("5 kB", FileSystem::Algorithm::Format(givenScaledSize, {}));
    EXPECT_EQ("5 kB", FileSystem::Algorithm::Format(givenScaledSize, 1));
    EXPECT_EQ("5 kB", FileSystem::Algorithm::Format(givenScaledSize, 0));
}

TEST(FileSystemAlgorithmTest, FormatFloatScaledFileSize) {
    const FileSystem::Algorithm::ScaledFileSize givenScaledSize{5.132812f, "kB"};
    EXPECT_EQ("5.132812 kB", FileSystem::Algorithm::Format(givenScaledSize, {}));
    EXPECT_EQ("5.13 kB", FileSystem::Algorithm::Format(givenScaledSize, 1));
    EXPECT_EQ("5.1 kB", FileSystem::Algorithm::Format(givenScaledSize, 0));
}

} // namespace FileSystemAlgorithmTests