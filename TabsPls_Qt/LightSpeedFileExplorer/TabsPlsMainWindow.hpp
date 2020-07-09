#pragma once

#include <memory>

#include <QMainWindow>

#include <TabsPlsCore/RobustDirectoryHistoryStore.hpp>

class CurrentDirectoryFileOp;

class TabsPlsMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	TabsPlsMainWindow(const QString& initialDirectory);

private:
	RobustDirectoryHistoryStore m_historyStore;
	std::shared_ptr<CurrentDirectoryFileOp> m_currentDirFileOpImpl;
};