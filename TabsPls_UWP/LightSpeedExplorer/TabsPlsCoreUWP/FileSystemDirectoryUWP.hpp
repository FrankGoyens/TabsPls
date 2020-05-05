#pragma once

#include <TabsPlsCore/FileSystemDirectory.hpp>

namespace winrt::Windows::Storage
{
	struct StorageFolder;
}

namespace FileSystem
{
	class DirectoryUWP : public Directory
	{
	public:
		static DirectoryUWP FromUWPStorageFolder(const winrt::Windows::Storage::StorageFolder& folder);

	private:
		DirectoryUWP(RawPath);
	};
}
