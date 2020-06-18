#include "FileListTableView.hpp"

#include <QHeaderView>
#include <QMouseEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QDebug>

FileListTableView::FileListTableView()
{
	verticalHeader()->hide();
	setShowGrid(false);
	setAcceptDrops(true);
}

void FileListTableView::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
		m_dragStartPosition = event->pos();

	QTableView::mousePressEvent(event);
}

void FileListTableView::mouseMoveEvent(QMouseEvent* event)
{
	if (!(event->buttons() & Qt::LeftButton))
	{
		QTableView::mouseMoveEvent(event);
		return;
	}

	if ((event->pos() - m_dragStartPosition).manhattanLength()
		< QApplication::startDragDistance())
		return;

	QDrag drag(this);
	QMimeData* mimeData = new QMimeData;

	mimeData->setData("text/uri-list", "file:///C:/file.txt");
	drag.setMimeData(mimeData);

	Qt::DropAction dropAction = drag.exec(Qt::CopyAction);
}

static void AcceptEventIfFormatIsOK(QDragMoveEvent& event)
{
	if (event.mimeData()->hasFormat("text/uri-list"))
		event.acceptProposedAction();
}

void FileListTableView::dragEnterEvent(QDragEnterEvent* event)
{
	AcceptEventIfFormatIsOK(*event);
}

void FileListTableView::dragMoveEvent(QDragMoveEvent* event)
{
	AcceptEventIfFormatIsOK(*event);
}

void FileListTableView::dropEvent(QDropEvent* event)
{
	qDebug() << event->mimeData()->text();
}
