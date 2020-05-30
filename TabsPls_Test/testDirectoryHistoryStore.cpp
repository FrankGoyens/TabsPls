#include <gtest/gtest.h>

#include <TabsPlsCore/DirectoryHistoryStore.hpp>

#include "FakeFileSystem.hpp"

namespace DirectoryHistoryStoreTests
{

    struct DirectoryHistoryStoreTest : ::testing::Test
    {
        ~DirectoryHistoryStoreTest() { FakeFileSystem::Cleanup(); }
    };

    TEST_F(DirectoryHistoryStoreTest, GetCurrentThrowsWhenStoreIsEmpty)
    {
        DirectoryHistoryStore givenStore;

        ASSERT_THROW(givenStore.GetCurrent().path(), StoreIsEmptyException);
    }

    TEST_F(DirectoryHistoryStoreTest, NewDirectoryBecomesCurrentDirectory)
    {
        FakeFileSystem::AddDirectory({ "C:", "users"});

        DirectoryHistoryStore givenStore;
        const auto givenFirstDirectory = FileSystem::Directory::FromPath(FakeFileSystem::MergeUsingSeparator({ "C:", "users" }));
        givenStore.OnNewDirectory(*givenFirstDirectory);

        ASSERT_EQ(givenFirstDirectory->path(), givenStore.GetCurrent().path());
    }

    TEST_F(DirectoryHistoryStoreTest, SwitchToPrevious)
    {
        FakeFileSystem::AddDirectory({ "C:", "users" });
        FakeFileSystem::AddDirectory({ "C:", "users", "jeff" });

        DirectoryHistoryStore givenStore;

        const auto givenFirstDirectory = FileSystem::Directory::FromPath(FakeFileSystem::MergeUsingSeparator({ "C:", "users" }));
        givenStore.OnNewDirectory(*givenFirstDirectory);
        
        const auto givenSecondDirectory = FileSystem::Directory::FromPath(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff" }));
        givenStore.OnNewDirectory(*givenSecondDirectory);

        ASSERT_EQ(givenSecondDirectory->path(), givenStore.GetCurrent().path());
        givenStore.SwitchToPrevious();
        ASSERT_EQ(givenFirstDirectory->path(), givenStore.GetCurrent().path());
        ASSERT_THROW(givenStore.SwitchToPrevious(), ImpossibleSwitchException);
    }

    TEST_F(DirectoryHistoryStoreTest, SwitchToNext)
    {
        FakeFileSystem::AddDirectory({ "C:", "users" });
        FakeFileSystem::AddDirectory({ "C:", "users", "jeff" });

        DirectoryHistoryStore givenStore;

        const auto givenFirstDirectory = FileSystem::Directory::FromPath(FakeFileSystem::MergeUsingSeparator({ "C:", "users" }));
        givenStore.OnNewDirectory(*givenFirstDirectory);

        const auto givenSecondDirectory = FileSystem::Directory::FromPath(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff" }));
        givenStore.OnNewDirectory(*givenSecondDirectory);

        givenStore.SwitchToPrevious();
        givenStore.SwitchToNext();
        ASSERT_EQ(givenSecondDirectory->path(), givenStore.GetCurrent().path());
        ASSERT_THROW(givenStore.SwitchToNext(), ImpossibleSwitchException);
    }
}
