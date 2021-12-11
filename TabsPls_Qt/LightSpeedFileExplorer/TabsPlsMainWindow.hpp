#pragma once

#include <memory>

#include <QMainWindow>

#include <TabsPlsCore/RobustDirectoryHistoryStore.hpp>

class CurrentDirectoryFileOp;

namespace TabModel {
struct Tab;
}

class TabsPlsMainWindow : public QMainWindow {
    Q_OBJECT

  public:
    TabsPlsMainWindow(const QString& initialDirectory);

  private:
    std::vector<std::shared_ptr<TabModel::Tab>> m_tabs;
};