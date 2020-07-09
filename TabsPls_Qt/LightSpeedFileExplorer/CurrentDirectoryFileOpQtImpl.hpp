#pragma once

#include <QObject>

#include <TabsPlsCore/CurrentDirectoryFileOp.hpp>
#include <TabsPlsCore/FileSystemDirectory.hpp>

class CurrentDirectoryFileOpQtImpl : public QObject, public CurrentDirectoryFileOp
{
	Q_OBJECT
public:
	CurrentDirectoryFileOpQtImpl(FileSystem::Directory currentDir);

	void updateCurrentDir(FileSystem::Directory);

	FileSystem::Directory GetCurrentDir() const override;

private:
	FileSystem::Directory m_currentDir;
};