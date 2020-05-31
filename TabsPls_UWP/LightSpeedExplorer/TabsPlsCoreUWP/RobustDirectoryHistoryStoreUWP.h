#pragma once

#include <RobustDirectoryHistoryStore.g.h>

#include <memory>

#include <winrt/Windows.Foundation.h>

namespace winrt::Windows::Storage
{
    struct StorageFolder;
}

class RobustDirectoryHistoryStore;

namespace winrt::TabsPlsCoreUWP::implementation
{
    class RobustDirectoryHistoryStore : public RobustDirectoryHistoryStoreT<RobustDirectoryHistoryStore>
    {
    public:
        RobustDirectoryHistoryStore();
        ~RobustDirectoryHistoryStore();

        void OnNewDirectory(const winrt::Windows::Storage::StorageFolder& storageFolder) const;

        bool SwitchToNext();
        bool SwitchToPrevious();

        winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::StorageFolder> GetCurrent() const;
    private:
        std::unique_ptr<::RobustDirectoryHistoryStore> m_storeImpl;
    };
}

namespace winrt::TabsPlsCoreUWP::factory_implementation
{
    struct RobustDirectoryHistoryStore : RobustDirectoryHistoryStoreT<RobustDirectoryHistoryStore, implementation::RobustDirectoryHistoryStore>
    {
    };
}
