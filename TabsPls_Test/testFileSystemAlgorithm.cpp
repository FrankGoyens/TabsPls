#include <gtest/gtest.h>

#include <TabsPlsCore/FileSystemAlgorithm.hpp>

#include "FakeFileSystem.hpp"

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

constexpr char* arbitraryDateString = "Mon Feb 15 16:17:18 2021";

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
} // namespace FileSystemAlgorithmTests