#include "pch.h"

#include <sstream>

#include <winrt/Windows.Storage.h>

#include <TabsPlsCore/FileSystem.hpp>
#include <TabsPlsCore/FileSystemFilePath.hpp>
#include <TabsPlsCore/FileSystemDirectory.hpp>

using namespace winrt::Windows::Storage;

namespace FileSystem
{
	bool IsDirectory(const RawPath& path)
	{
		try
		{
			StorageFolder::GetFolderFromPathAsync(path).get();
			return true;
		}catch(const winrt::hresult_error&){}
		return false;
	}

	bool IsRegularFile(const RawPath& path)
	{
		try
		{
			StorageFile::GetFileFromPathAsync(path).get();
			return true;
		}
		catch (const winrt::hresult_error&) {}
		return false;
	}

	static std::wstring ExePath() {
		wchar_t buffer[MAX_PATH];
		GetModuleFileName(NULL, buffer, MAX_PATH);
		std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
		return std::wstring(buffer).substr(0, pos);
	}

	RawPath GetWorkingDirectory()
	{
		return ExePath();
	}

	RawPath RemoveFilename(const FilePath& filePath) 
	{
		auto storageFile = StorageFile::GetFileFromPathAsync(filePath.path()).get();
		return storageFile.GetParentAsync().get().Path().data();
	}

	RawPath GetParent(const Directory& dir)
	{
		auto storageFolder = StorageFolder::GetFolderFromPathAsync(dir.path()).get();
		return storageFolder.GetParentAsync().get().Path().data();
	}
}
