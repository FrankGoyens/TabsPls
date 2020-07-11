#include "TabsPlsMainWindow.hpp"

#include <stdexcept>

#include <QShortcut>
#include <QTabWidget>

#include "FileBrowserWidget.hpp"

int CreateNewFileBrowserTab(QTabWidget& tabWidget, FileSystem::Directory dir)
{

	auto* tab = new FileBrowserWidget(std::move(dir));
	const int tabIndex = tabWidget.addTab(tab, tab->GetCurrentDirectoryName());

	QWidget::connect(tab, &FileBrowserWidget::currentDirectoryNameChanged, [&, tabIndex](const auto& newName) {tabWidget.setTabText(tabIndex, newName); });
	return tabIndex;
}

TabsPlsMainWindow::TabsPlsMainWindow(const QString& initialDirectory)
{
	setWindowTitle(tr("Light Speed File Explorer"));

	const auto validInitialDir = FileSystem::Directory::FromPath(initialDirectory.toStdString());

	if (!validInitialDir)
		throw std::invalid_argument("The given directory does not exist");

	auto* tabWidget = new QTabWidget();
	tabWidget->setTabBarAutoHide(true);
	
	CreateNewFileBrowserTab(*tabWidget, *validInitialDir);

	const auto* createTabShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_T), this);
	connect(createTabShortcut, &QShortcut::activated, [=]() 
	{
		if (const auto* fileBrowserTab = dynamic_cast<FileBrowserWidget*>(tabWidget->currentWidget())) {
			const int newIndex = CreateNewFileBrowserTab(*tabWidget, fileBrowserTab->GetCurrentDirectory());
			tabWidget->setCurrentIndex(newIndex);
		}
	});

	const auto* closeTabShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this);
	connect(closeTabShortcut, &QShortcut::activated, [=]()
	{
		tabWidget->removeTab(tabWidget->currentIndex());
	});

	setCentralWidget(tabWidget);
}