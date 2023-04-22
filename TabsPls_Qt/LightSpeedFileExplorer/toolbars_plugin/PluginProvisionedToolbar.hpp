#pragma once

#include <TabsPlsCore/Toolbar.hpp>

#include <QToolBar>

class PluginProvisionedToolbar : public QToolBar {
    Q_OBJECT
  public:
    PluginProvisionedToolbar(TabsPlsPython::Toolbar::Toolbar toolbarModel);

  signals:
    void RequestChangeDirectory(QString newDirectory);
    void RequestOpenDirectoryInNewTab(QString newDirectory);

  private:
    TabsPlsPython::Toolbar::Toolbar m_toolbarModel;

    void HandleResultingRequest(const TabsPlsPython::Toolbar::ActivationResult&);
};