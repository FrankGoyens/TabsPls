#include "pch.h"
#include "DirectoryHistoryStoreUWP.h"
#include <DirectoryHistoryStore.g.cpp>

#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Foundation.h>

#include <FileSystemDirectoryUWP.hpp>

#include <TabsPlsCore/DirectoryHistoryStore.hpp>

using winrt::Windows::Storage::StorageFolder;
using winrt::Windows::Foundation::IAsyncOperation;

namespace winrt::TabsPlsCoreUWP::implementation
{
	DirectoryHistoryStore::DirectoryHistoryStore() :
		m_storeImpl(std::make_unique<::DirectoryHistoryStore>())
	{
	}

	DirectoryHistoryStore::~DirectoryHistoryStore() = default;

	void DirectoryHistoryStore::OnNewDirectory(const StorageFolder& storageFolder) const
	{
		m_storeImpl->OnNewDirectory(FileSystem::DirectoryUWP::FromUWPStorageFolder(storageFolder));
	}

	bool DirectoryHistoryStore::SwitchToPrevious()
	{
		try
		{
			m_storeImpl->SwitchToPrevious();
			return true;
		}
		catch (const ImpossibleSwitchException&) {}
		return false;
	}

	bool DirectoryHistoryStore::SwitchToNext()
	{
		try
		{
			m_storeImpl->SwitchToNext();
			return true;
		}
		catch (const ImpossibleSwitchException&) {}
		return false;
	}

	IAsyncOperation<StorageFolder> DirectoryHistoryStore::GetCurrent() const
	{
		return StorageFolder::GetFolderFromPathAsync(m_storeImpl->GetCurrent().path().c_str());
	}
}
