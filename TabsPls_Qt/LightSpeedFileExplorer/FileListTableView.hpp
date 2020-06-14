#pragma once

#include <QTableView>

class QMouseEvent;
class QDragEnterEvent;
class QDropEvent;

class FileListTableView: public QTableView
{
	Q_OBJECT

public:
	FileListTableView();

protected:
	void mousePressEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
	void dragEnterEvent(QDragEnterEvent*) override;
	void dragMoveEvent(QDragMoveEvent*) override;
	void dropEvent(QDropEvent*) override;

private:
	QPoint m_dragStartPosition;
};