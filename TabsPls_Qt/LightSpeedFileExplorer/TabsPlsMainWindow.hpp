#pragma once

#include <QMainWindow>

#include <TabsPlsCore/RobustDirectoryHistoryStore.hpp>

class TabsPlsMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	TabsPlsMainWindow(const QString& initialDirectory);

private:
	RobustDirectoryHistoryStore m_historyStore;
};