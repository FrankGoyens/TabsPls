#pragma once

#include <TabsPlsCore/FileSystemDirectory.hpp>

class CurrentDirectoryFileOp
{
public:
	virtual ~CurrentDirectoryFileOp() = default;

	//Throw the same exceptions as FileSystem::Op::CopyRecursive
	void CopyRecursive(const FileSystem::RawPath& source, const FileSystem::Name& destName);

protected:
	virtual FileSystem::Directory GetCurrentDir() const = 0;
};