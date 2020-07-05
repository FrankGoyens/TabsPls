#include <gtest/gtest.h>

#include <TabsPlsCore/FileSystemDirectory.hpp>
#include <TabsPlsCore/FileSystemFilePath.hpp>

#include <TabsPlsCore/FileSystemOp.hpp>

#include "../FakeFileSystem.hpp"

namespace FakeFileSystemMetaTest
{

	struct FakeFileSystemTest : ::testing::Test
	{
		~FakeFileSystemTest() { FakeFileSystem::Cleanup(); }
	};

	TEST_F(FakeFileSystemTest, NoFileInEmptyFS)
	{
		EXPECT_FALSE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({ "C:", "file.txt" })));
	}

	TEST_F(FakeFileSystemTest, NoDirectoryInEmptyFS)
	{
		EXPECT_FALSE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({ "C:" })));
	}

	TEST_F(FakeFileSystemTest, IsDirectoryEmptyString)
	{
		EXPECT_FALSE(FileSystem::IsDirectory(""));
	}

	TEST_F(FakeFileSystemTest, AddDirectory)
	{
		FakeFileSystem::AddDirectory({ "C:" });
		EXPECT_TRUE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({ "C:" })));
	}

	TEST_F(FakeFileSystemTest, AddDirectoryFailsWithEmptyArgument)
	{
		FakeFileSystem::AddDirectory({ });
		EXPECT_FALSE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({ "C:" })));
	}

	TEST_F(FakeFileSystemTest, IsRegularFileEmptyString)
	{
		EXPECT_FALSE(FileSystem::IsRegularFile(""));
	}

	TEST_F(FakeFileSystemTest, AddFile)
	{
		FakeFileSystem::AddFile({ "C:", "users", "jeff" }, "file.txt");
		EXPECT_TRUE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff" , "file.txt"})));
	}

	TEST_F(FakeFileSystemTest, AddFileFailsWithoutHavingParentDirectory)
	{
		FakeFileSystem::AddFile({ }, "file.txt");
		EXPECT_FALSE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({ "C:", "file.txt" })));
	}

	TEST_F(FakeFileSystemTest, CreateDirectory)
	{
		FakeFileSystem::AddDirectory({ "C:", "users", "jeff" });
		const auto createdDirectory = FileSystem::Directory::FromPath(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff"}));
		EXPECT_TRUE(createdDirectory.has_value());
		EXPECT_EQ(createdDirectory->path(), FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff"}));
	}

	TEST_F(FakeFileSystemTest, CreateFilePath)
	{
		FakeFileSystem::AddFile({ "C:", "users", "jeff" }, "file.txt");
		const auto createdFilePath = FileSystem::FilePath::FromPath(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff", "file.txt" }));
		EXPECT_TRUE(createdFilePath.has_value());
		EXPECT_EQ(createdFilePath->path(), FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff", "file.txt" }));
	}

	TEST_F(FakeFileSystemTest, GetDirectoryName)
	{
		FakeFileSystem::AddDirectory({ "C:", "users", "jeff" });
		const auto createdDirectory = FileSystem::Directory::FromPath(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff" }));
		EXPECT_EQ(FileSystem::GetDirectoryname(*createdDirectory), "jeff");
	}

	TEST_F(FakeFileSystemTest, GetFileName)
	{
		FakeFileSystem::AddFile({ "C:", "users", "jeff" }, "file.txt");
		const auto createdFilePath = FileSystem::FilePath::FromPath(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff", "file.txt" }));
		EXPECT_EQ(FileSystem::GetFilename(*createdFilePath), "file.txt");
	}

	TEST_F(FakeFileSystemTest, RemovedFileName)
	{
		FakeFileSystem::AddFile({ "C:", "users", "jeff" }, "file.txt");
		const auto createdFilePath = FileSystem::FilePath::FromPath(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff", "file.txt" }));
		EXPECT_EQ(FileSystem::RemoveFilename(*createdFilePath), FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff" }));
	}

	TEST_F(FakeFileSystemTest, _getRootPath)
	{
		EXPECT_EQ(FileSystem::_getRootPath(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff", "file.txt" })), std::string("C:") + FakeFileSystem::GetSeparator());
	}

	TEST_F(FakeFileSystemTest, _getRootName)
	{
		EXPECT_EQ(FileSystem::_getRootName(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff", "file.txt" })), "C:");
	}

	static void FillJeffDirectory()
	{
		FakeFileSystem::AddFile({ "C:", "users", "jeff" }, "file1.txt");
		FakeFileSystem::AddFile({ "C:", "users", "jeff" }, "file2.txt");
		FakeFileSystem::AddFile({ "C:", "users", "jeff" }, "file3.txt");
		FakeFileSystem::AddFile({ "C:", "users", "jeff" }, "file4.txt");
		FakeFileSystem::AddDirectory({ "C:", "users", "jeff", "documents" });
		FakeFileSystem::AddDirectory({ "C:", "users", "jeff", "photos" });
	}

	TEST_F(FakeFileSystemTest, GetFilesInDirectory)
	{
		FillJeffDirectory();
		const auto createdDirectory = FileSystem::Directory::FromPath(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff" }));
		EXPECT_TRUE(createdDirectory.has_value());

		const std::vector<FileSystem::Name> expectedContents = {"file1.txt", "file2.txt" , "file3.txt" , "file4.txt" , "documents" , "photos" };
		EXPECT_TRUE(std::is_permutation(expectedContents.begin(), expectedContents.end(), FileSystem::GetFilesInDirectory(*createdDirectory).begin()));
	}

	TEST_F(FakeFileSystemTest, GetParent)
	{
		FakeFileSystem::AddDirectory({ "C:", "users", "jeff" });
		const auto createdDirectory = FileSystem::Directory::FromPath(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff" }));
		EXPECT_EQ(FileSystem::GetParent(*createdDirectory), FakeFileSystem::MergeUsingSeparator({ "C:", "users" }));

		const auto createdRootDirectory = FileSystem::Directory::FromPath(FakeFileSystem::MergeUsingSeparator({ "C:" }));
		EXPECT_EQ(FileSystem::GetParent(*createdRootDirectory), FakeFileSystem::MergeUsingSeparator({ "C:"}));
	}

	TEST_F(FakeFileSystemTest, DeleteDirectory)
	{
		EXPECT_FALSE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff" })));
		FileSystem::Op::RemoveAll(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff" }));

		EXPECT_FALSE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff" })));
		FakeFileSystem::AddDirectory({ "C:", "users", "jeff" });

		EXPECT_TRUE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff" })));
		FileSystem::Op::RemoveAll(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff" }));

		EXPECT_FALSE(FileSystem::IsDirectory(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff" })));
	}

	TEST_F(FakeFileSystemTest, DeleteFile)
	{
		EXPECT_FALSE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff", "file.txt" })));
		FileSystem::Op::RemoveAll(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff", "file.txt" }));

		EXPECT_FALSE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff", "file.txt" })));
		FakeFileSystem::AddFile({ "C:", "users", "jeff" }, "file.txt");

		EXPECT_TRUE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff", "file.txt" })));
		FileSystem::Op::RemoveAll(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff" , "file.txt" }));

		EXPECT_FALSE(FileSystem::IsRegularFile(FakeFileSystem::MergeUsingSeparator({ "C:", "users", "jeff", "file.txt" })));
	}
}
