#include <gtest/gtest.h>

#include <TabsPlsCore/FileSystemAlgorithm.hpp>

#include "FakeFileSystem.hpp"

namespace FileSystemAlgorithmTests
{
	TEST(FileSystemAlgorithmTest, StripTrailingPathSeparators)
	{
		auto givenPath = FakeFileSystem::MergeUsingSeparator({ "C", "", "" });
		EXPECT_EQ(FileSystem::Algorithm::StripTrailingPathSeparators(givenPath), "C");

		givenPath = "C";
		EXPECT_EQ(FileSystem::Algorithm::StripTrailingPathSeparators(givenPath), "C");

		givenPath = FakeFileSystem::MergeUsingSeparator({ "C", "Users", "..", "Users", ".", "Jeff", "", "", "" });
		EXPECT_EQ(FileSystem::Algorithm::StripTrailingPathSeparators(givenPath), FakeFileSystem::MergeUsingSeparator({ "C", "Users", "..", "Users", ".", "Jeff" }));
	}

	TEST(FileSystemAlgorithmTest, StripLeadingPathSeparators)
	{
		auto givenPath = FakeFileSystem::MergeUsingSeparator({ "", "", "C" });
		EXPECT_EQ(FileSystem::Algorithm::StripLeadingPathSeparators(givenPath), "C");

		givenPath = "C";
		EXPECT_EQ(FileSystem::Algorithm::StripLeadingPathSeparators(givenPath), "C");

		givenPath = FakeFileSystem::MergeUsingSeparator({ "", "", "", "C", "Users", "..", "Users", ".", "Jeff", });
		EXPECT_EQ(FileSystem::Algorithm::StripLeadingPathSeparators(givenPath), FakeFileSystem::MergeUsingSeparator({ "C", "Users", "..", "Users", ".", "Jeff" }));
	}
}