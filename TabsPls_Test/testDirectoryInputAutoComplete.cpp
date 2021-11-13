#include <gtest/gtest.h>

#include <TabsPlsCore/DirectoryInputAutoComplete.hpp>

#include "FakeFileSystem.hpp"

struct TestDirectoryInputAutoComplete : ::testing::Test {
    ~TestDirectoryInputAutoComplete() { FakeFileSystem::Cleanup(); }
};

TEST_F(TestDirectoryInputAutoComplete, Do) {
    ASSERT_FALSE(DirectoryInputAutoComplete::Do(FakeFileSystem::MergeUsingSeparator({"C:", "users", "jeff"})));

    FakeFileSystem::AddDirectory({"C:", "users", "jeff"});
    FakeFileSystem::AddDirectory({"C:", "users", "jack"});

    const auto createdAutoCompleter =
        DirectoryInputAutoComplete::Do(FakeFileSystem::MergeUsingSeparator({"C:", "users", "je"}));

    ASSERT_TRUE(createdAutoCompleter);
    EXPECT_EQ(FakeFileSystem::MergeUsingSeparator({"C:", "users", "jeff"}), *createdAutoCompleter);

    const auto createdSecondAutoCompleter =
        DirectoryInputAutoComplete::Do(FakeFileSystem::MergeUsingSeparator({"C:", "users", "jeff"}));
    ASSERT_TRUE(createdSecondAutoCompleter);
    EXPECT_EQ(FakeFileSystem::MergeUsingSeparator({"C:", "users", "jack"}), *createdSecondAutoCompleter);
}

TEST_F(TestDirectoryInputAutoComplete, DoReverse) {
    ASSERT_FALSE(DirectoryInputAutoComplete::Do(FakeFileSystem::MergeUsingSeparator({"C:", "users", "jeff"})));

    FakeFileSystem::AddDirectory({"C:", "users", "jeff"});
    FakeFileSystem::AddDirectory({"C:", "users", "jack"});

    const auto createdAutoCompleter =
        DirectoryInputAutoComplete::DoReverse(FakeFileSystem::MergeUsingSeparator({"C:", "users", "je"}));

    ASSERT_TRUE(createdAutoCompleter);
    EXPECT_EQ(FakeFileSystem::MergeUsingSeparator({"C:", "users", "jack"}), *createdAutoCompleter);

    const auto createdSecondAutoCompleter =
        DirectoryInputAutoComplete::DoReverse(FakeFileSystem::MergeUsingSeparator({"C:", "users", "jack"}));
    ASSERT_TRUE(createdSecondAutoCompleter);
    EXPECT_EQ(FakeFileSystem::MergeUsingSeparator({"C:", "users", "jeff"}), *createdSecondAutoCompleter);
}