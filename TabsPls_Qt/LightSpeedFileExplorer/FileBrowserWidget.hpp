#pragma once

#include <memory>

#include <QFileSystemWatcher>
#include <QWidget>

#include <TabsPlsCore/RobustDirectoryHistoryStore.hpp>
#include <TabsPlsCore/Toolbar.hpp>

namespace FileSystem {
class Directory;
}

class CurrentDirectoryFileOpQtImpl;
class FileListTableViewWithFilter;

class FileBrowserWidget : public QWidget {
    Q_OBJECT
  public:
    FileBrowserWidget(FileSystem::Directory initialDir,
                      const std::vector<TabsPlsPython::Toolbar::Toolbar>& pluginToolbars);

    const QString GetCurrentDirectoryName() const;
    const FileSystem::Directory& GetCurrentDirectory() const { return m_currentDirectory; }

    void RequestChangeToFlatDirectoryStructure();
    void RequestChangeToHierarchyDirectoryStructure();

  public slots:
    void RequestSetCurrentDirectoryToPrevious();
    void RequestSetCurrentDirectoryToNext();

  signals:
    void currentDirectoryNameChanged(const QString&);
    void RequestOpenDirectoryInTab(const FileSystem::Directory&);

  private:
    FileSystem::Directory m_currentDirectory;
    RobustDirectoryHistoryStore m_historyStore;
    std::shared_ptr<CurrentDirectoryFileOpQtImpl> m_currentDirFileOpImpl;
    QFileSystemWatcher m_fs_watcher;
    FileListTableViewWithFilter* m_fileListTableView;

    std::function<void()> m_backAction;
    std::function<void()> m_forwardAction;

    void SetCurrentDirectory(FileSystem::Directory);

    void StartWatchingCurrentDirectory();
    void StopWatchingCurrentDirectory();

    template <typename Func> void DisplayDirectoryChangedErrorIfExceptionHappens(Func);
    void DisplayDirectoryChangedError(const char* message);
};
