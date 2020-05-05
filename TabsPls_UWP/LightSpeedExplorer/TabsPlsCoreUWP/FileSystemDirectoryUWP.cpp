#include "pch.h"
#include "FileSystemDirectoryUWP.hpp"

#include <winrt/Windows.Storage.h>

using winrt::Windows::Storage::StorageFolder;

namespace FileSystem
{
	DirectoryUWP FileSystem::DirectoryUWP::FromUWPStorageFolder(const winrt::Windows::Storage::StorageFolder& folder)
	{
		return DirectoryUWP(folder.Path().data());
	}

	DirectoryUWP::DirectoryUWP(RawPath rawPath):
		Directory(std::move(rawPath))
	{}
}
