#include "pch.h"
#include "FilePathUWP.hpp"

#include <winrt/Windows.Storage.h>

namespace FileSystem
{
	FilePathUWP FilePathUWP::FromUWPStorageFile(const winrt::Windows::Storage::StorageFile& storageFile)
	{
		return FilePathUWP(storageFile.Path().data());
	}

	FilePathUWP::FilePathUWP(RawPath rawPath):
		FilePath(std::move(rawPath))
	{

	}
}
