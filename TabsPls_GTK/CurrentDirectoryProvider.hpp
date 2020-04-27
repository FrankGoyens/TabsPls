#pragma once

namespace FileSystem
{
	class Directory;
}

struct CurrentDirectoryProvider
{
	virtual ~CurrentDirectoryProvider() = default;
	virtual const FileSystem::Directory& Get() const = 0;
};


