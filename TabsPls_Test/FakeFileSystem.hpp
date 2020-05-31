#pragma once

#include <TabsPlsCore/FileSystem.hpp>

namespace FakeFileSystem
{
	/*! \brief this completely resets the fake file system. Useful to call during a test teardown*/
	void Cleanup();

	void AddDirectory(const std::initializer_list<FileSystem::Name>& absoluteComponents);
	void AddFile(const std::initializer_list<FileSystem::Name>& parentAbsoluteComponents, const FileSystem::Name& fileName);

	void DeleteDirectory(const std::initializer_list<FileSystem::Name>& absoluteComponents);
	void DeleteFile(const std::initializer_list<FileSystem::Name>& parentAbsoluteComponents, const FileSystem::Name& fileName);

	FileSystem::RawPath MergeUsingSeparator(const std::vector<FileSystem::Name>& components);

	FileSystem::Name GetSeparator();
}