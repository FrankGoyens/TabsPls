#pragma once

#include <memory>

#include <QFileSystemWatcher>
#include <QWidget>

#include <TabsPlsCore/RobustDirectoryHistoryStore.hpp>

namespace FileSystem {
class Directory;
}

class CurrentDirectoryFileOpQtImpl;

class FileBrowserWidget : public QWidget {
    Q_OBJECT
  public:
    FileBrowserWidget(FileSystem::Directory initialDir);

    const QString GetCurrentDirectoryName() const;
    const FileSystem::Directory& GetCurrentDirectory() const { return m_currentDirectory; }

  signals:
    void currentDirectoryNameChanged(const QString&);

  private:
    FileSystem::Directory m_currentDirectory;
    RobustDirectoryHistoryStore m_historyStore;
    std::shared_ptr<CurrentDirectoryFileOpQtImpl> m_currentDirFileOpImpl;
    QFileSystemWatcher m_fs_watcher;

    void SetCurrentDirectory(FileSystem::Directory);

    void StartWatchingCurrentDirectory();
    void StopWatchingCurrentDirectory();

    template <typename Func> void DisplayDirectoryChangedErrorIfExceptionHappens(Func);
    void DisplayDirectoryChangedError(const char* message);
};
