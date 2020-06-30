#include "FileListTableView.hpp"

#include <QHeaderView>
#include <QMouseEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QDebug>
#include <QLineEdit>

FileListTableView::FileListTableView()
{
	verticalHeader()->hide();
	setShowGrid(false);
	setAcceptDrops(true);
}

int FileListTableView::GetModelRoleForFullPaths()
{
	return Qt::UserRole;
}

int FileListTableView::GetModelRoleForNames()
{
	return Qt::DisplayRole;
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

	mimeData->setData("text/uri-list", AggregateSelectionDataForDrag().toUtf8());
	drag.setMimeData(mimeData);

	drag.exec(Qt::CopyAction);
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

QString FileListTableView::AggregateSelectionDataForDrag() const
{
	const auto selectionIndices = selectionModel()->selectedRows();
	QStringList dataAsList;
	for (const auto& index : selectionIndices)
	{
		const QString filePath = model()->data(index, Qt::UserRole).toString();
		dataAsList << QUrl::fromLocalFile(filePath).toString();
	}
	return dataAsList.join('\n');
}
