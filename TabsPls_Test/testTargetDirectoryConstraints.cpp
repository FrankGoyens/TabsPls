#include <gtest/gtest.h>

#include <FileSystemDefs.hpp>

#include <TabsPlsCore/TargetDirectoryConstraints.hpp>

namespace TargetDirectoryConstraints {

bool DirIsRoot(const FileSystem::RawPath& dir);
FileSystem::RawPath RemoveWhitespace(FileSystem::RawPath string);

} // namespace TargetDirectoryConstraints

#if defined(_WIN32) || defined(WIN32)

TEST(testTargetDirectoryConstraints, DirIsRoot) {
    EXPECT_TRUE(TargetDirectoryConstraints::DirIsRoot({"C:"}));
    EXPECT_TRUE(TargetDirectoryConstraints::DirIsRoot({"C:\\"}));
}

TEST(testTargetDirectoryConstraints, IsIncompleteWindowsRootPath) {
    EXPECT_TRUE(TargetDirectoryConstraints::IsIncompleteWindowsRootPath({"C:"}));
    EXPECT_TRUE(TargetDirectoryConstraints::IsIncompleteWindowsRootPath({"C:\t  "}));
    EXPECT_FALSE(TargetDirectoryConstraints::IsIncompleteWindowsRootPath({"C:\\"}));
    EXPECT_FALSE(TargetDirectoryConstraints::IsIncompleteWindowsRootPath({"C:\\\t  "}));
}

#endif

TEST(testTargetDirectoryConstraints, RemoveWhitespace) {
    EXPECT_EQ(std::string("abc"), TargetDirectoryConstraints::RemoveWhitespace("a b c"));
    EXPECT_EQ(std::string("abc"), TargetDirectoryConstraints::RemoveWhitespace("a \tb\n\n c    "));
}