#include <gtest/gtest.h>

#include <TabsPlsCore/FileSystemDirectory.hpp>
#include <TabsPlsCore/FileSystemOp.hpp>

#include "FakeFileSystem.hpp"

namespace CreateNewDirectoryTests {
struct CreateNewDirectoryTest : ::testing::Test {
    ~CreateNewDirectoryTest() { FakeFileSystem::Cleanup(); }
};

TEST_F(CreateNewDirectoryTest, CreateDirectory) {
    FakeFileSystem::AddDirectory({"C:", "users"});

    FileSystem::Op::CreateDirectory(
        *FileSystem::Directory::FromPath(FakeFileSystem::MergeUsingSeparator({"C:", "users"})), "Jos");

    EXPECT_TRUE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "users", "Jos"})));
}

TEST_F(CreateNewDirectoryTest, CreateDirectoryWithUnsanitizedName) {
    FakeFileSystem::AddDirectory({"C:", "users"});

    FileSystem::Op::CreateDirectory(
        *FileSystem::Directory::FromPath(FakeFileSystem::MergeUsingSeparator({"C:", "users"})),
        FileSystem::Separator() + FileSystem::Separator() + "Jos");

    EXPECT_TRUE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "users", "Jos"})));
}
} // namespace CreateNewDirectoryTests
