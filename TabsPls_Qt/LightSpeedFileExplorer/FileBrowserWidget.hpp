#pragma once

#include <memory>

#include <QWidget>

#include <TabsPlsCore/RobustDirectoryHistoryStore.hpp>

namespace FileSystem
{
	class Directory;
}

class CurrentDirectoryFileOp;

class FileBrowserWidget : public QWidget
{
	Q_OBJECT
public:
	FileBrowserWidget(const FileSystem::Directory& initialDir);

private:
	RobustDirectoryHistoryStore m_historyStore;
	std::shared_ptr<CurrentDirectoryFileOp> m_currentDirFileOpImpl;
};