#include <gtest/gtest.h>

#include <TabsPlsCore/RobustDirectoryHistoryStore.hpp>

#include "FakeFileSystem.hpp"

namespace RobustDirectoryHistoryStoreTests
{

    struct RobustDirectoryHistoryStoreTest : ::testing::Test
    {
        ~RobustDirectoryHistoryStoreTest() { FakeFileSystem::Cleanup(); }
    };

    TEST_F(RobustDirectoryHistoryStoreTest, OnNewDirectoryTrustsDirectoryExistance)
    {
        FakeFileSystem::AddDirectory({ "C:", "users", "jeff" });

        RobustDirectoryHistoryStore givenStore;
        const auto givenDirectory = FileSystem::Directory::FromPath(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff" }));
        
        FakeFileSystem::DeleteDirectory({ "C:", "users", "jeff" });
        givenStore.OnNewDirectory(*givenDirectory);
    }

    TEST_F(RobustDirectoryHistoryStoreTest, SwitchToPrevious)
    {
        FakeFileSystem::AddDirectory({ "C:", "users", "jeff" });

        RobustDirectoryHistoryStore givenStore;
        const auto givenFirstDirectory = FileSystem::Directory::FromPath(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff" }));
        givenStore.OnNewDirectory(*givenFirstDirectory);
        const auto givenSecondDirectory = FileSystem::Directory::FromPath(FakeFileSystem::MergeUsingSeparator({ "C:", "users" }));
        givenStore.OnNewDirectory(*givenSecondDirectory);

        FakeFileSystem::DeleteDirectory({ "C:", "users", "jeff" });
        givenStore.SwitchToPrevious();
        EXPECT_EQ(givenStore.GetCurrent().path(), FakeFileSystem::MergeUsingSeparator({ "C:", "users" })); //SwitchToPrevious() was a no-op
    }

    TEST_F(RobustDirectoryHistoryStoreTest, SwitchToNext)
    {
        FakeFileSystem::AddDirectory({ "C:", "users", "jeff" });

        RobustDirectoryHistoryStore givenStore;
        const auto givenFirstDirectory = FileSystem::Directory::FromPath(FakeFileSystem::MergeUsingSeparator({ "C:", "users" }));
        givenStore.OnNewDirectory(*givenFirstDirectory);
        const auto givenSecondDirectory = FileSystem::Directory::FromPath(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff" }));
        givenStore.OnNewDirectory(*givenSecondDirectory);
        givenStore.SwitchToPrevious();

        FakeFileSystem::DeleteDirectory({ "C:", "users", "jeff" });
        givenStore.SwitchToNext();
        EXPECT_EQ(givenStore.GetCurrent().path(), FakeFileSystem::MergeUsingSeparator({ "C:", "users" })); //SwitchToPrevious() was a no-op
    }
}
