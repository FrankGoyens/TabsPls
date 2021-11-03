#include "TabsPlsMainWindow.hpp"

#include <stdexcept>

#include <QAction>
#include <QDesktopServices>
#include <QMenuBar>
#include <QShortcut>
#include <QTabWidget>
#include <QUrl>

#include "FileBrowserWidget.hpp"
#include "FileSystemDefsConversion.hpp"

using FileSystem::StringConversion::FromRawPath;
using FileSystem::StringConversion::ToRawPath;

int CreateNewFileBrowserTab(QTabWidget& tabWidget, FileSystem::Directory dir) {

    auto* tab = new FileBrowserWidget(std::move(dir));
    const int tabIndex = tabWidget.addTab(tab, tab->GetCurrentDirectoryName());
    const QString namePrefix = tabIndex < 9 ? "&" + QString::number(tabIndex + 1) + " " : "";

    tabWidget.setTabText(tabIndex, namePrefix + tabWidget.tabText(tabIndex));

    QWidget::connect(
        tab, &FileBrowserWidget::currentDirectoryNameChanged,
        [&, tabIndex, namePrefix](const auto& newName) { tabWidget.setTabText(tabIndex, namePrefix + newName); });
    return tabIndex;
}

static void SetupMenubar(QMenuBar& menubar, QMainWindow& mainWindow, QTabWidget& tabWidget) {
    auto* quit = new QAction("&Quit", &menubar);

    auto* file = menubar.addMenu("&File");
    file->addAction(quit);

    QObject::connect(quit, &QAction::triggered, [&] { mainWindow.close(); });

    auto* openInExplorer = new QAction("&Explorer");
    auto* openIn = menubar.addMenu("&Open in");
    openIn->addAction(openInExplorer);

    QObject::connect(openInExplorer, &QAction::triggered, [&, openInExplorer] {
        if (const auto* currentFileBrowser = dynamic_cast<FileBrowserWidget*>(tabWidget.currentWidget())) {
            QDesktopServices::openUrl(
                QUrl::fromLocalFile(FromRawPath(currentFileBrowser->GetCurrentDirectory().path())));
        }
    });
}

TabsPlsMainWindow::TabsPlsMainWindow(const QString& initialDirectory) {
    setWindowTitle(tr("Light Speed File Explorer"));
    setWindowIcon(QIcon(":boot_icon_32.png"));

    const auto validInitialDir = FileSystem::Directory::FromPath(ToRawPath(initialDirectory));

    if (!validInitialDir)
        throw std::invalid_argument("The given directory does not exist");

    auto* tabWidget = new QTabWidget();
    tabWidget->setTabBarAutoHide(true);

    CreateNewFileBrowserTab(*tabWidget, *validInitialDir);

    const auto* createTabShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_T), this);
    connect(createTabShortcut, &QShortcut::activated, [=]() {
        if (const auto* fileBrowserTab = dynamic_cast<FileBrowserWidget*>(tabWidget->currentWidget())) {
            const int newIndex = CreateNewFileBrowserTab(*tabWidget, fileBrowserTab->GetCurrentDirectory());
            tabWidget->setCurrentIndex(newIndex);
        }
    });

    const auto* closeTabShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this);
    connect(closeTabShortcut, &QShortcut::activated, [=]() {
        if (tabWidget->count() > 1)
            tabWidget->removeTab(tabWidget->currentIndex());
    });

    setCentralWidget(tabWidget);

    SetupMenubar(*menuBar(), *this, *tabWidget);
}
