#pragma once

#include <TabsPlsCore/FileSystemFilePath.hpp>

namespace winrt::Windows::Storage
{
	struct StorageFile;
}

namespace FileSystem
{
	class FilePathUWP: public FilePath
	{
	public:
		static FilePathUWP FromUWPStorageFile(const winrt::Windows::Storage::StorageFile&);
	private:
		FilePathUWP(RawPath);
	};
}