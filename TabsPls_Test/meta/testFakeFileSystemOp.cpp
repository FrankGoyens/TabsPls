#include <gtest/gtest.h>

#include <TabsPlsCore/FileSystemOp.hpp>

#include "../FakeFileSystem.hpp"

namespace FakeFileSystemOpTests {

struct FakeFileSystemOpTest : ::testing::Test {
    ~FakeFileSystemOpTest() { FakeFileSystem::Cleanup(); }
};

TEST_F(FakeFileSystemOpTest, RenameFile) {
    FakeFileSystem::AddFile({"C:", "Users", "Jef"}, "A.txt");

    EXPECT_TRUE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "A.txt"})));
    EXPECT_FALSE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "B.txt"})));

    FileSystem::Op::Rename(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "A.txt"}),
                           FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "B.txt"}));

    EXPECT_FALSE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "A.txt"})));
    EXPECT_TRUE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "B.txt"})));
}

TEST_F(FakeFileSystemOpTest, RenameDir) {
    FakeFileSystem::AddDirectory({"C:", "Users", "Jef"});

    EXPECT_TRUE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"})));
    EXPECT_FALSE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Geoff"})));

    FileSystem::Op::Rename(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"}),
                           FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Geoff"}));

    EXPECT_FALSE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"})));
    EXPECT_TRUE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Geoff"})));
}

TEST_F(FakeFileSystemOpTest, RenameDirKeepsChildren) {
    FakeFileSystem::AddFile({"C:", "Users", "Jef"}, "NOTES.txt");
    FakeFileSystem::AddFile({"C:", "Users", "Jef", "Pictures"}, "Waterfall.png");

    FileSystem::Op::Rename(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"}),
                           FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Geoff"}));

    EXPECT_TRUE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Geoff", "Pictures"})));
    EXPECT_TRUE(FileSystem::IsRegularFile(
        FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Geoff", "Pictures", "Waterfall.png"})));
    EXPECT_TRUE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Geoff", "NOTES.txt"})));
}

TEST_F(FakeFileSystemOpTest, RenameWithoutSource) {
    EXPECT_FALSE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "A.txt"})));

    EXPECT_THROW(FileSystem::Op::Rename(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "A.txt"}),
                                        FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "B.txt"})),
                 FileSystem::Op::RenameException);
}

TEST_F(FakeFileSystemOpTest, RenameDirWithSameDest) {
    FakeFileSystem::AddDirectory({"C:", "Users", "Jef"});

    EXPECT_TRUE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"})));
    FileSystem::Op::Rename(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"}),
                           FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"}));
    EXPECT_TRUE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"})));
}

TEST_F(FakeFileSystemOpTest, RenameFileWithSameDest) {
    FakeFileSystem::AddFile({"C:", "Users", "Jef"}, "NOTES.txt");

    EXPECT_TRUE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "NOTES.txt"})));
    FileSystem::Op::Rename(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "NOTES.txt"}),
                           FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "NOTES.txt"}));
    EXPECT_TRUE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "NOTES.txt"})));
}

TEST_F(FakeFileSystemOpTest, RenameDirWithExisingDirDest) {
    FakeFileSystem::AddDirectory({"C:", "Users", "Jef"});
    FakeFileSystem::AddDirectory({"C:", "Users", "Geoff"});

    EXPECT_THROW(FileSystem::Op::Rename(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"}),
                                        FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Geoff"})),
                 FileSystem::Op::RenameException);
}

TEST_F(FakeFileSystemOpTest, RenameDirWithExisingFileDest) {
    FakeFileSystem::AddDirectory({"C:", "Users", "Jef"});
    FakeFileSystem::AddFile({"C:", "Users"}, "Geoff");

    EXPECT_THROW(FileSystem::Op::Rename(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"}),
                                        FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Geoff"})),
                 FileSystem::Op::RenameException);
}

TEST_F(FakeFileSystemOpTest, RenameFileWithExisingDirDest) {
    FakeFileSystem::AddFile({"C:", "Users"}, "Jef");
    FakeFileSystem::AddDirectory({"C:", "Users", "Geoff"});

    EXPECT_THROW(FileSystem::Op::Rename(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"}),
                                        FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Geoff"})),
                 FileSystem::Op::RenameException);
}

TEST_F(FakeFileSystemOpTest, RenameFileWithExisingFileDest) {
    FakeFileSystem::AddFile({"C:", "Users"}, "Jef");
    FakeFileSystem::AddFile({"C:", "Users"}, "Geoff");

    EXPECT_THROW(FileSystem::Op::Rename(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"}),
                                        FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Geoff"})),
                 FileSystem::Op::RenameException);
}

TEST_F(FakeFileSystemOpTest, RenameRoot) {
    FakeFileSystem::AddDirectory({"C:"});

    FileSystem::Op::Rename("C:",
                           "D:"); // This is not expected to work on an actual filesystem,
                                  // but there are no write restrictions on the FakeFileSystem

    EXPECT_FALSE(FileSystem::IsDirectory("C:"));
    EXPECT_TRUE(FileSystem::IsDirectory("D:"));
}

TEST_F(FakeFileSystemOpTest, CopyFile) {
    FakeFileSystem::AddFile({"C:", "Users", "Jef"}, "A.txt");

    EXPECT_TRUE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "A.txt"})));
    EXPECT_FALSE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "B.txt"})));

    FileSystem::Op::CopyRecursive(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "A.txt"}),
                                  FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "B.txt"}));

    EXPECT_TRUE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "A.txt"})));
    EXPECT_TRUE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "B.txt"})));
}

TEST_F(FakeFileSystemOpTest, CopyDir) {
    FakeFileSystem::AddDirectory({"C:", "Users", "Jef"});

    EXPECT_TRUE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"})));
    EXPECT_FALSE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"D:", "Users", "Geoff"})));

    FileSystem::Op::CopyRecursive(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"}),
                                  FakeFileSystem::MergeUsingSeparator({"D:", "Users", "Geoff"}));

    EXPECT_TRUE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"})));
    EXPECT_TRUE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"D:", "Users", "Geoff"})));
}

TEST_F(FakeFileSystemOpTest, CopyDirKeepsChildren) {
    FakeFileSystem::AddFile({"C:", "Users", "Jef"}, "NOTES.txt");
    FakeFileSystem::AddFile({"C:", "Users", "Jef", "Pictures"}, "Waterfall.png");

    FileSystem::Op::CopyRecursive(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"}),
                                  FakeFileSystem::MergeUsingSeparator({"D:", "Users", "Geoff"}));

    EXPECT_TRUE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "Pictures"})));
    EXPECT_TRUE(FileSystem::IsRegularFile(
        FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "Pictures", "Waterfall.png"})));
    EXPECT_TRUE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "NOTES.txt"})));

    EXPECT_TRUE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"D:", "Users", "Geoff", "Pictures"})));
    EXPECT_TRUE(FileSystem::IsRegularFile(
        FakeFileSystem::MergeUsingSeparator({"D:", "Users", "Geoff", "Pictures", "Waterfall.png"})));
    EXPECT_TRUE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"D:", "Users", "Geoff", "NOTES.txt"})));
}

TEST_F(FakeFileSystemOpTest, CopyWithoutSource) {
    EXPECT_FALSE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "A.txt"})));

    EXPECT_THROW(FileSystem::Op::CopyRecursive(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "A.txt"}),
                                               FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "B.txt"})),
                 FileSystem::Op::CopyException);
}

TEST_F(FakeFileSystemOpTest, CopyDirWithSameDest) {
    FakeFileSystem::AddDirectory({"C:", "Users", "Jef"});

    EXPECT_TRUE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"})));
    EXPECT_THROW(FileSystem::Op::CopyRecursive(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"}),
                                               FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"})),
                 FileSystem::Op::CopyException);
    EXPECT_TRUE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"})));
}

TEST_F(FakeFileSystemOpTest, CopyFileWithSameDest) {
    FakeFileSystem::AddFile({"C:", "Users", "Jef"}, "NOTES.txt");

    EXPECT_TRUE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "NOTES.txt"})));
    EXPECT_THROW(
        FileSystem::Op::CopyRecursive(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "NOTES.txt"}),
                                      FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "NOTES.txt"})),
        FileSystem::Op::CopyException);
    EXPECT_TRUE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef", "NOTES.txt"})));
}

TEST_F(FakeFileSystemOpTest, CopyDirWithExisingDirDest) {
    FakeFileSystem::AddDirectory({"C:", "Users", "Jef"});
    FakeFileSystem::AddDirectory({"C:", "Users", "Geoff"});

    EXPECT_THROW(FileSystem::Op::CopyRecursive(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"}),
                                               FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Geoff"})),
                 FileSystem::Op::CopyException);
}

TEST_F(FakeFileSystemOpTest, CopyDirWithExisingFileDest) {
    FakeFileSystem::AddDirectory({"C:", "Users", "Jef"});
    FakeFileSystem::AddFile({"C:", "Users"}, "Geoff");

    EXPECT_THROW(FileSystem::Op::CopyRecursive(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"}),
                                               FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Geoff"})),
                 FileSystem::Op::CopyException);
}

TEST_F(FakeFileSystemOpTest, CopyFileWithExisingDirDest) {
    FakeFileSystem::AddFile({"C:", "Users"}, "Jef");
    FakeFileSystem::AddDirectory({"C:", "Users", "Geoff"});

    /*This one might seem odd, but if the intent is to copy the file 'Jef' into
    C:\Users\Geoff, Then the command should be 'CopyRecursive(C:\Users\Jef,
    C:\Users\Geoff\Jef)'
    */

    EXPECT_THROW(FileSystem::Op::CopyRecursive(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"}),
                                               FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Geoff"})),
                 FileSystem::Op::CopyException);
}

TEST_F(FakeFileSystemOpTest, CopyFileWithExisingFileDest) {
    FakeFileSystem::AddFile({"C:", "Users"}, "Jef");
    FakeFileSystem::AddFile({"C:", "Users"}, "Geoff");

    EXPECT_THROW(FileSystem::Op::CopyRecursive(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Jef"}),
                                               FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Geoff"})),
                 FileSystem::Op::CopyException);
}

TEST_F(FakeFileSystemOpTest, CopyRoot) {
    FakeFileSystem::AddDirectory({"C:"});

    FileSystem::Op::CopyRecursive("C:",
                                  "D:"); // This is not expected to work on an actual filesystem,
                                         // but there are no write restrictions on the FakeFileSystem

    EXPECT_TRUE(FileSystem::IsDirectory("C:"));
    EXPECT_TRUE(FileSystem::IsDirectory("D:"));
}

TEST_F(FakeFileSystemOpTest, RemoveDir) {
    FakeFileSystem::AddDirectory({"C:", "Users", "Dan"});
    EXPECT_TRUE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Dan"})));

    FileSystem::Op::RemoveAll(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Dan"}));
    EXPECT_FALSE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Dan"})));
}

TEST_F(FakeFileSystemOpTest, RemoveNonExistingDir) {
    EXPECT_FALSE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Dan"})));
    FileSystem::Op::RemoveAll(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Dan"}));
    EXPECT_FALSE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Dan"})));
}

TEST_F(FakeFileSystemOpTest, RemoveDirAlsoRemovesChildren) {
    FakeFileSystem::AddDirectory({"C:", "Users", "Dan", "Pictures"});
    FakeFileSystem::AddFile({"C:", "Users", "Dan"}, "NOTES.txt");

    EXPECT_TRUE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Dan"})));
    EXPECT_TRUE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Dan", "Pictures"})));
    EXPECT_TRUE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Dan", "NOTES.txt"})));

    FileSystem::Op::RemoveAll(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Dan"}));

    EXPECT_FALSE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Dan"})));
    EXPECT_FALSE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Dan", "Pictures"})));
    EXPECT_FALSE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Dan", "NOTES.txt"})));
}

TEST_F(FakeFileSystemOpTest, RemoveFile) {
    FakeFileSystem::AddFile({"C:", "Users", "Dan"}, "NOTES.txt");
    EXPECT_TRUE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Dan", "NOTES.txt"})));

    FileSystem::Op::RemoveAll(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Dan", "NOTES.txt"}));
    EXPECT_FALSE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Dan", "NOTES.txt"})));
}

TEST_F(FakeFileSystemOpTest, RemoveNonExistingFile) {
    EXPECT_FALSE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Dan", "NOTES.txt"})));
    FileSystem::Op::RemoveAll(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Dan", "NOTES.txt"}));
    EXPECT_FALSE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({"C:", "Users", "Dan", "NOTES.txt"})));
}
} // namespace FakeFileSystemOpTests