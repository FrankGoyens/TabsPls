#include "pch.h"
#include "RobustDirectoryHistoryStoreUWP.h"
#include <RobustDirectoryHistoryStore.g.cpp>

#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Foundation.h>

#include <FileSystemDirectoryUWP.hpp>

#include <TabsPlsCore/RobustDirectoryHistoryStore.hpp>

using winrt::Windows::Storage::StorageFolder;
using winrt::Windows::Foundation::IAsyncOperation;

namespace winrt::TabsPlsCoreUWP::implementation
{
	RobustDirectoryHistoryStore::RobustDirectoryHistoryStore() :
		m_storeImpl(std::make_unique<::RobustDirectoryHistoryStore>())
	{
	}

	RobustDirectoryHistoryStore::~RobustDirectoryHistoryStore() = default;

	void RobustDirectoryHistoryStore::OnNewDirectory(const StorageFolder& storageFolder) const
	{
		m_storeImpl->OnNewDirectory(FileSystem::DirectoryUWP::FromUWPStorageFolder(storageFolder));
	}

	bool RobustDirectoryHistoryStore::SwitchToPrevious()
	{
		try
		{
			m_storeImpl->SwitchToPrevious();
			return true;
		}
		catch (const ImpossibleSwitchException&) {}
		return false;
	}

	bool RobustDirectoryHistoryStore::SwitchToNext()
	{
		try
		{
			m_storeImpl->SwitchToNext();
			return true;
		}
		catch (const ImpossibleSwitchException&) {}
		return false;
	}

	IAsyncOperation<StorageFolder> RobustDirectoryHistoryStore::GetCurrent() const
	{
		return StorageFolder::GetFolderFromPathAsync(m_storeImpl->GetCurrent().path().c_str());
	}
}
