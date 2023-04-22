#pragma once

#include <memory>

#include <QMainWindow>

#include <TabsPlsCore/RobustDirectoryHistoryStore.hpp>
#include <TabsPlsCore/Toolbar.hpp>

class CurrentDirectoryFileOp;

namespace TabModel {
struct Tab;
}

class TabsPlsMainWindow : public QMainWindow {
    Q_OBJECT

  public:
    TabsPlsMainWindow(const QString& initialDirectory);

  private:
    QTabWidget* m_tabWidget = nullptr;
    std::vector<std::shared_ptr<TabModel::Tab>> m_tabs;
    std::vector<TabsPlsPython::Toolbar::Toolbar> m_toolbarModels;

    std::shared_ptr<TabModel::Tab> OpenNewTab(const FileSystem::Directory&);
};