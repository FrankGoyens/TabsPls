#include "TabsPlsMainWindow.hpp"

#include <stdexcept>

#include <QAction>
#include <QDesktopServices>
#include <QMenuBar>
#include <QShortcut>
#include <QTabWidget>
#include <QUrl>

#include <TabsPlsCore/TabModel.hpp>

#include "FileBrowserWidget.hpp"
#include "FileSystemDefsConversion.hpp"

using FileSystem::StringConversion::FromRawPath;
using FileSystem::StringConversion::ToRawPath;

static QString AsQString(const TabModel::TabLabel& label) {
    return label.mnemonic ? QString("&").append(*label.mnemonic).append(" ").append(label.label.c_str())
                          : QString(label.label.c_str());
}

void ConnectDirectoryChangedSignalToTab(const std::weak_ptr<TabModel::Tab>& tab,
                                        const FileBrowserWidget& widgetWithinTab, QTabWidget& tabWidget) {
    QWidget::connect(&widgetWithinTab, &FileBrowserWidget::currentDirectoryNameChanged, [&, tab](const auto& newName) {
        if (const auto liveTab = tab.lock()) {
            liveTab->name = newName.toStdString();
            tabWidget.setTabText(liveTab->index, AsQString(TabModel::LabelFromTabModel(*liveTab)));
        }
    });
}

std::shared_ptr<TabModel::Tab> CreateNewFileBrowserTab(QTabWidget& tabWidget, FileSystem::Directory dir) {

    auto* widgetWithinTab = new FileBrowserWidget(std::move(dir));
    const auto tabName = widgetWithinTab->GetCurrentDirectoryName();
    const int tabIndex = tabWidget.addTab(widgetWithinTab, tabName);

    const auto tabModel = std::make_shared<TabModel::Tab>(TabModel::Tab{tabIndex, tabName.toStdString()});

    tabWidget.setTabText(tabIndex, AsQString(LabelFromTabModel(*tabModel)));

    ConnectDirectoryChangedSignalToTab(tabModel, *widgetWithinTab, tabWidget);
    return tabModel;
}

static void SetupMenubar_View(QMenuBar& menubar, QMainWindow& mainWindow, QTabWidget& tabWidget) {
    auto* hierarchyMode = new QAction("Switch to &Hierarchy");
    auto* flatMode = new QAction("Switch to &Flat");

    auto* view = menubar.addMenu("&View");
    view->addAction(hierarchyMode);
    view->addAction(flatMode);

    QObject::connect(hierarchyMode, &QAction::triggered, [&] {
        if (auto* currentFileBrowser = dynamic_cast<FileBrowserWidget*>(tabWidget.currentWidget())) {
            currentFileBrowser->RequestChangeToHierarchyDirectoryStructure();
        }
    });

    QObject::connect(flatMode, &QAction::triggered, [&] {
        if (auto* currentFileBrowser = dynamic_cast<FileBrowserWidget*>(tabWidget.currentWidget())) {
            currentFileBrowser->RequestChangeToFlatDirectoryStructure();
        }
    });
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

    SetupMenubar_View(menubar, mainWindow, tabWidget);
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
            TabModel::ReassignTabIndices(m_tabs);
            TabModel::ReassignTabLabels(
                m_tabs, [tabWidget](int index, const auto& label) { tabWidget->setTabText(index, AsQString(label)); });
        }
    });

    setCentralWidget(tabWidget);

    SetupMenubar(*menuBar(), *this, *tabWidget);
}
