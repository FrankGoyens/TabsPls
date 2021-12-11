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

QString NamePrefixFromIndex(int index) { return index < 9 ? "&" + QString::number(index + 1) + " " : ""; }

QString LabelFromTabModel(const TabsPlsMainWindow::Tab& tabModel) {
    return NamePrefixFromIndex(tabModel.index) + tabModel.name;
}

void ConnectDirectoryChangedSignalToTab(const std::weak_ptr<TabsPlsMainWindow::Tab>& tab,
                                        const FileBrowserWidget& widgetWithinTab, QTabWidget& tabWidget) {
    QWidget::connect(&widgetWithinTab, &FileBrowserWidget::currentDirectoryNameChanged, [&, tab](const auto& newName) {
        if (const auto liveTab = tab.lock()) {
            liveTab->name = newName;
            tabWidget.setTabText(liveTab->index, LabelFromTabModel(*liveTab));
        }
    });
}

std::shared_ptr<TabsPlsMainWindow::Tab> CreateNewFileBrowserTab(QTabWidget& tabWidget, FileSystem::Directory dir) {

    auto* widgetWithinTab = new FileBrowserWidget(std::move(dir));
    const auto tabName = widgetWithinTab->GetCurrentDirectoryName();
    const int tabIndex = tabWidget.addTab(widgetWithinTab, tabName);

    const auto tabModel = std::make_shared<TabsPlsMainWindow::Tab>(TabsPlsMainWindow::Tab{tabIndex, tabName});

    tabWidget.setTabText(tabIndex, LabelFromTabModel(*tabModel));

    ConnectDirectoryChangedSignalToTab(tabModel, *widgetWithinTab, tabWidget);
    return tabModel;
}

static void SetupMenubar(QMenuBar& menubar, QMainWindow& mainWindow, QTabWidget& tabWidget) {
    auto* quit = new QAction("&Quit", &menubar);

    auto* file = menubar.addMenu("&File");
    file->addAction(quit);

    QObject::connect(quit, &QAction::triggered, [&] { mainWindow.close(); });

    auto* openInExplorer = new QAction("&Explorer");
    auto* openIn = menubar.addMenu("&Open in");
    openIn->addAction(openInExplorer);

    QObject::connect(openInExplorer, &QAction::triggered, [&] {
        if (const auto* currentFileBrowser = dynamic_cast<FileBrowserWidget*>(tabWidget.currentWidget())) {
            QDesktopServices::openUrl(
                QUrl::fromLocalFile(FromRawPath(currentFileBrowser->GetCurrentDirectory().path())));
        }
    });
}

static void ReassignTabIndices(const std::vector<std::shared_ptr<TabsPlsMainWindow::Tab>>& tabs) {
    for (int tabIndex = 0; tabIndex < tabs.size(); ++tabIndex) {
        tabs[tabIndex]->index = tabIndex;
    }
}

static void ReassignTabLabels(const std::vector<std::shared_ptr<TabsPlsMainWindow::Tab>>& tabs, QTabWidget& tabWidget) {
    for (int tabIndex = 0; tabIndex < tabs.size(); ++tabIndex) {
        tabWidget.setTabText(tabIndex, LabelFromTabModel(*tabs[tabIndex]));
    }
}

TabsPlsMainWindow::TabsPlsMainWindow(const QString& initialDirectory) {
    setWindowTitle(tr("Light Speed File Explorer"));
    setWindowIcon(QIcon(":boot_icon_32.png"));

    const auto validInitialDir = FileSystem::Directory::FromPath(ToRawPath(initialDirectory));

    if (!validInitialDir)
        throw std::invalid_argument("The given directory does not exist");

    auto* tabWidget = new QTabWidget();
    tabWidget->setTabBarAutoHide(true);

    m_tabs.push_back(CreateNewFileBrowserTab(*tabWidget, *validInitialDir));

    const auto* createTabShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_T), this);
    connect(createTabShortcut, &QShortcut::activated, [=]() {
        if (const auto* fileBrowserTab = dynamic_cast<FileBrowserWidget*>(tabWidget->currentWidget())) {
            const auto tabModel = CreateNewFileBrowserTab(*tabWidget, fileBrowserTab->GetCurrentDirectory());
            m_tabs.push_back(tabModel);
            tabWidget->setCurrentIndex(tabModel->index);
        }
    });

    const auto* closeTabShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this);
    connect(closeTabShortcut, &QShortcut::activated, [=]() {
        if (tabWidget->count() > 1) {
            const auto removalIndex = tabWidget->currentIndex();
            tabWidget->removeTab(removalIndex);
            m_tabs.erase(m_tabs.begin() + removalIndex);
            ReassignTabIndices(m_tabs);
            ReassignTabLabels(m_tabs, *tabWidget);
        }
    });

    setCentralWidget(tabWidget);

    SetupMenubar(*menuBar(), *this, *tabWidget);
}
