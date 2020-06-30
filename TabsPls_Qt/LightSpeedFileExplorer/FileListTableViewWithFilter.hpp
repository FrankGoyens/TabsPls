#pragma once

#include <QWidget>

class FileListTableView;
class QLineEdit;

class FileListTableViewWithFilter: public QWidget
{
	Q_OBJECT

public:
	FileListTableViewWithFilter();

	FileListTableView& GetFileListTableView() { return *m_fileListTableView; }

private:
	FileListTableView* m_fileListTableView;
	QLineEdit* m_filterField;
};