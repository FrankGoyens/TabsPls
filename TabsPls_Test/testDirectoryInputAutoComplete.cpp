#include <gtest/gtest.h>

#include <TabsPlsCore/DirectoryInputAutoComplete.hpp>

#include "FakeFileSystem.hpp"

struct TestDirectoryInputAutoComplete : ::testing::Test {
    ~TestDirectoryInputAutoComplete() { FakeFileSystem::Cleanup(); }
};

TEST_F(TestDirectoryInputAutoComplete, Do) {
    ASSERT_FALSE(DirectoryInputAutoComplete::Do(FakeFileSystem::MergeUsingSeparator({"C:", "users", "jeff"})));

    FakeFileSystem::AddDirectory({"C:", "users", "jeff"});

    const auto createdAutoCompleter =
        DirectoryInputAutoComplete::Do(FakeFileSystem::MergeUsingSeparator({"C:", "users", "je"}));

    ASSERT_TRUE(createdAutoCompleter);
    EXPECT_EQ(FakeFileSystem::MergeUsingSeparator({"C:", "users", "jeff"}), *createdAutoCompleter);
}