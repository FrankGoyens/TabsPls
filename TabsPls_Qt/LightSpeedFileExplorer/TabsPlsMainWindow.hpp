#pragma once

#include <memory>

#include <QMainWindow>

#include <TabsPlsCore/RobustDirectoryHistoryStore.hpp>

class CurrentDirectoryFileOp;

class TabsPlsMainWindow : public QMainWindow {
    Q_OBJECT

  public:
    TabsPlsMainWindow(const QString& initialDirectory);

    struct Tab {
        int index;
        QString name;
    };

  private:
    std::vector<std::shared_ptr<Tab>> m_tabs;
};