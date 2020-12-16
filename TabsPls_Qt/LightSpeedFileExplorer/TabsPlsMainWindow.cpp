#include "TabsPlsMainWindow.hpp"

#include <stdexcept>

#include <QShortcut>
#include <QTabWidget>

#include "FileBrowserWidget.hpp"
#include "FileSystemDefsConversion.hpp"

using FileSystem::StringConversion::ToRawPath;

int CreateNewFileBrowserTab(QTabWidget& tabWidget, FileSystem::Directory dir) {

    auto* tab = new FileBrowserWidget(std::move(dir));
    const int tabIndex = tabWidget.addTab(tab, tab->GetCurrentDirectoryName());
    const QString namePrefix =
        tabIndex < 9 ? "&" + QString::number(tabIndex + 1) + " " : "";

    tabWidget.setTabText(tabIndex, namePrefix + tabWidget.tabText(tabIndex));

    QWidget::connect(tab, &FileBrowserWidget::currentDirectoryNameChanged,
                     [&, tabIndex, namePrefix](const auto& newName) {
                         tabWidget.setTabText(tabIndex, namePrefix + newName);
                     });
    return tabIndex;
}

TabsPlsMainWindow::TabsPlsMainWindow(const QString& initialDirectory) {
    setWindowTitle(tr("Light Speed File Explorer"));

    const auto validInitialDir =
        FileSystem::Directory::FromPath(ToRawPath(initialDirectory));

    if (!validInitialDir)
        throw std::invalid_argument("The given directory does not exist");

    auto* tabWidget = new QTabWidget();
    tabWidget->setTabBarAutoHide(true);

    CreateNewFileBrowserTab(*tabWidget, *validInitialDir);

    const auto* createTabShortcut =
        new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_T), this);
    connect(createTabShortcut, &QShortcut::activated, [=]() {
        if (const auto* fileBrowserTab =
                dynamic_cast<FileBrowserWidget*>(tabWidget->currentWidget())) {
            const int newIndex = CreateNewFileBrowserTab(
                *tabWidget, fileBrowserTab->GetCurrentDirectory());
            tabWidget->setCurrentIndex(newIndex);
        }
    });

    const auto* closeTabShortcut =
        new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this);
    connect(closeTabShortcut, &QShortcut::activated, [=]() {
        if (tabWidget->count() > 1)
            tabWidget->removeTab(tabWidget->currentIndex());
    });

    setCentralWidget(tabWidget);
}
