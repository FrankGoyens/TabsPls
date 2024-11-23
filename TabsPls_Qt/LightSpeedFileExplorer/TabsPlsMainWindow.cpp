#include "TabsPlsMainWindow.hpp"

#include <stdexcept>

#include <QAction>
#include <QDesktopServices>
#include <QMenuBar>
#include <QMouseEvent>
#include <QShortcut>
#include <QTabBar>
#include <QTabWidget>
#include <QUrl>

#include <TabsPlsCore/TabModel.hpp>

#include "FileBrowserWidget.hpp"
#include "FileSystemDefsConversion.hpp"

using FileSystem::StringConversion::FromRawPath;
using FileSystem::StringConversion::ToRawPath;

struct TabContainterItemImpl : TabModel::TabContainerItem {
    TabContainterItemImpl(std::shared_ptr<TabModel::Tab> tab, QMetaObject::Connection directoryChangedSignalConnection)
        : tab(std::move(tab)), directoryChangedSignalConnection(std::move(directoryChangedSignalConnection)) {
        if (!this->tab)
            abort();
    }

    TabModel::Tab& GetTab() { return *tab; }

    std::shared_ptr<TabModel::Tab> tab;
    QMetaObject::Connection directoryChangedSignalConnection;
};

static QString AsQString(const TabModel::TabLabel& label) {
    return label.mnemonic ? QString("&").append(*label.mnemonic).append(" ").append(label.label.c_str())
                          : QString(label.label.c_str());
}

static QMetaObject::Connection ConnectDirectoryChangedSignalToTab(const std::weak_ptr<TabModel::Tab>& tab,
                                                                  const FileBrowserWidget& widgetWithinTab,
                                                                  QTabWidget& tabWidget) {
    auto connection = QObject::connect(
        &widgetWithinTab, &FileBrowserWidget::currentDirectoryNameChanged, [&, tab](const auto& newName) {
            if (const auto liveTab = tab.lock()) {
                liveTab->name = newName.toStdString();
                tabWidget.setTabText(liveTab->index, AsQString(TabModel::LabelFromTabModel(*liveTab)));
            }
        });
    return connection;
}

static std::pair<std::unique_ptr<TabContainterItemImpl>, FileBrowserWidget&>
CreateNewFileBrowserTab(QTabWidget& tabWidget, FileSystem::Directory dir,
                        const std::vector<TabsPlsPython::Toolbar::Toolbar>& toolbarModels) {

    auto* widgetWithinTab = new FileBrowserWidget(std::move(dir), toolbarModels);
    const auto tabName = widgetWithinTab->GetCurrentDirectoryName();
    const int tabIndex = tabWidget.addTab(widgetWithinTab, tabName);

    auto tabModel = std::make_shared<TabModel::Tab>(TabModel::Tab{tabIndex, tabName.toStdString()});

    tabWidget.setTabText(tabIndex, AsQString(LabelFromTabModel(*tabModel)));

    auto directoryChangedSignalConnection = ConnectDirectoryChangedSignalToTab(tabModel, *widgetWithinTab, tabWidget);
    return {std::make_unique<TabContainterItemImpl>(std::move(tabModel), std::move(directoryChangedSignalConnection)),
            *widgetWithinTab};
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

static void LoadToolbarModels(std::vector<TabsPlsPython::Toolbar::Toolbar>& toolbars) {
    if (!TabsPlsPython::Toolbar::ComponentIsAvailable())
        return;

    const auto pluginsDirectory = FileSystem::Directory::FromPath(
        FileSystem::Directory::FromCurrentWorkingDirectory().path() + FileSystem::Separator() + L"toolbars");
    if (!pluginsDirectory)
        return;

    auto loadedToolbars = TabsPlsPython::Toolbar::LoadToolbars(*pluginsDirectory);
    std::move(loadedToolbars.begin(), loadedToolbars.end(), std::back_inserter(toolbars));
}

static void ResetDirectoryChangedSignals(std::vector<std::unique_ptr<TabContainterItemImpl>>& tabs,
                                         QTabWidget& tabWidget) {
    for (auto& tabContainerItem : tabs) {
        QObject::disconnect(tabContainerItem->directoryChangedSignalConnection);
    }

    if (tabWidget.count() != tabs.size())
        abort();

    for (int i = 0; i < tabs.size(); ++i) {
        const auto* fileBrowserTab = dynamic_cast<FileBrowserWidget*>(tabWidget.widget(i));
        assert(fileBrowserTab);
        if (fileBrowserTab) {
            tabs[i]->directoryChangedSignalConnection =
                ConnectDirectoryChangedSignalToTab(tabs[i]->tab, *fileBrowserTab, tabWidget);
        }
    }
}

TabsPlsMainWindow::TabsPlsMainWindow(const QString& initialDirectory) {
    LoadToolbarModels(m_toolbarModels);

    setWindowTitle(tr("Light Speed File Explorer"));
    setWindowIcon(QIcon(":boot_icon_32.png"));

    const auto validInitialDir = FileSystem::Directory::FromPath(ToRawPath(initialDirectory));

    if (!validInitialDir)
        throw std::invalid_argument("The given directory does not exist");

    m_tabWidget = new QTabWidget();
    m_tabWidget->setTabBarAutoHide(true);
    m_tabWidget->setMovable(true);

    (void)OpenNewTab(*validInitialDir);

    const auto resetTabLabels = [tabWidget = m_tabWidget](int index, const auto& label) {
        tabWidget->setTabText(index, AsQString(label));
    };

    connect(m_tabWidget->tabBar(), &QTabBar::tabMoved, [=](int from, int to) {
        TabModel::SwapTabs(m_tabs, from, to);
        TabModel::ReassignTabLabels(m_tabs, resetTabLabels);

        ResetDirectoryChangedSignals(m_tabs, *m_tabWidget);
    });

    const auto* createTabShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_T), this);
    connect(createTabShortcut, &QShortcut::activated, [=]() {
        if (const auto* fileBrowserTab = dynamic_cast<FileBrowserWidget*>(m_tabWidget->currentWidget())) {
            auto& tabModel = OpenNewTab(fileBrowserTab->GetCurrentDirectory());
            m_tabWidget->setCurrentIndex(tabModel.GetTab().index);
        }
    });

    const auto* closeTabShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this);
    connect(closeTabShortcut, &QShortcut::activated, [=]() {
        if (m_tabWidget->count() > 1) {
            const auto removalIndex = m_tabWidget->currentIndex();
            m_tabWidget->removeTab(removalIndex);
            m_tabs.erase(m_tabs.begin() + removalIndex);
            TabModel::ReassignTabIndices(m_tabs);
            TabModel::ReassignTabLabels(m_tabs, resetTabLabels);
        }
    });

    setCentralWidget(m_tabWidget);

    SetupMenubar(*menuBar(), *this, *m_tabWidget);
}

TabsPlsMainWindow::~TabsPlsMainWindow() = default;

void TabsPlsMainWindow::mousePressEvent(QMouseEvent* mouseEvent) {
    if (mouseEvent->button() == Qt::BackButton || mouseEvent->button() == Qt::ForwardButton) {
        if (auto* fileBrowserTab = dynamic_cast<FileBrowserWidget*>(m_tabWidget->currentWidget())) {
            if (mouseEvent->button() == Qt::BackButton)
                fileBrowserTab->RequestSetCurrentDirectoryToPrevious();
            else
                fileBrowserTab->RequestSetCurrentDirectoryToNext();
        }
        mouseEvent->accept();
    }
    QMainWindow::mousePressEvent(mouseEvent);
}

TabContainterItemImpl& TabsPlsMainWindow::OpenNewTab(const FileSystem::Directory& directory) {
    auto [tabModelContainerItem, filebrowserWidget] = CreateNewFileBrowserTab(*m_tabWidget, directory, m_toolbarModels);
    m_tabs.push_back(std::move(tabModelContainerItem));
    connect(&filebrowserWidget, &FileBrowserWidget::RequestOpenDirectoryInTab,
            [this](const FileSystem::Directory& newDirectory) {
                auto& tabModel = OpenNewTab(newDirectory);
                m_tabWidget->setCurrentIndex(tabModel.GetTab().index);
            });
    return *m_tabs.back();
}
