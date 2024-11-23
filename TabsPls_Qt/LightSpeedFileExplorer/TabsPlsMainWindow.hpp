#pragma once

#include <memory>

#include <QMainWindow>

#include <TabsPlsCore/RobustDirectoryHistoryStore.hpp>
#include <TabsPlsCore/Toolbar.hpp>

class CurrentDirectoryFileOp;

namespace TabModel {
struct Tab;
}

struct TabContainterItemImpl;

class TabsPlsMainWindow : public QMainWindow {
    Q_OBJECT

  public:
    TabsPlsMainWindow(const QString& initialDirectory);
    ~TabsPlsMainWindow();

  protected:
    void mousePressEvent(QMouseEvent*) override;

  private:
    QTabWidget* m_tabWidget = nullptr;
    std::vector<std::unique_ptr<TabContainterItemImpl>> m_tabs;
    std::vector<TabsPlsPython::Toolbar::Toolbar> m_toolbarModels;

    TabContainterItemImpl& OpenNewTab(const FileSystem::Directory&);
};