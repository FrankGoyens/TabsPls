#pragma once

#include <DirectoryHistoryStore.g.h>

#include <memory>

#include <winrt/Windows.Foundation.h>

namespace winrt::Windows::Storage
{
    struct StorageFolder;
}

class DirectoryHistoryStore;

namespace winrt::TabsPlsCoreUWP::implementation
{
    class DirectoryHistoryStore : public DirectoryHistoryStoreT<DirectoryHistoryStore>
    {
    public:
        DirectoryHistoryStore();
        ~DirectoryHistoryStore();

        void OnNewDirectory(const winrt::Windows::Storage::StorageFolder& storageFolder) const;

        bool SwitchToNext();
        bool SwitchToPrevious();

        winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::StorageFolder> GetCurrent() const;
    private:
        std::unique_ptr<::DirectoryHistoryStore> m_storeImpl;
    };
}

namespace winrt::TabsPlsCoreUWP::factory_implementation
{
    struct DirectoryHistoryStore : DirectoryHistoryStoreT<DirectoryHistoryStore, implementation::DirectoryHistoryStore>
    {
    };
}
