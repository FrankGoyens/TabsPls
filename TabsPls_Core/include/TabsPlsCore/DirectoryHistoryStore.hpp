#pragma once

#include "FileSystemDirectory.hpp"
#include <stack>

struct ImpossibleSwitchException : std::exception {};
struct StoreIsEmptyException : std::exception {};

class DirectoryHistoryStore {
  public:
    DirectoryHistoryStore() = default;
    virtual ~DirectoryHistoryStore() = default;

    DirectoryHistoryStore(DirectoryHistoryStore&&) noexcept = default;
    DirectoryHistoryStore(const DirectoryHistoryStore&) = default;

    DirectoryHistoryStore& operator=(DirectoryHistoryStore) noexcept;

    void OnNewDirectory(const FileSystem::Directory&);

    void SwitchToPrevious();
    void SwitchToNext();

    const FileSystem::Directory GetCurrent() const;

    friend void swap(DirectoryHistoryStore& first,
                     DirectoryHistoryStore& second) noexcept;

  private:
    std::stack<FileSystem::Directory> m_previousDirs;
    std::stack<FileSystem::Directory> m_nextDirs;
};