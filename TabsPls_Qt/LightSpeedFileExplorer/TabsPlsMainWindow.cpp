#include "TabsPlsMainWindow.hpp"

#include <stdexcept>

#include "FileBrowserWidget.hpp"

TabsPlsMainWindow::TabsPlsMainWindow(const QString& initialDirectory)
{
	setWindowTitle(tr("Light Speed File Explorer"));

	const auto validInitialDir = FileSystem::Directory::FromPath(initialDirectory.toStdString());

	if (!validInitialDir)
		throw std::invalid_argument("The given directory does not exist");

	setCentralWidget(new FileBrowserWidget(*validInitialDir));
}