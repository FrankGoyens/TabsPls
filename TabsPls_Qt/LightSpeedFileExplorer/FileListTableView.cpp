#include "FileListTableView.hpp"

#include <stdexcept>

#include <QHeaderView>
#include <QMouseEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QDebug>
#include <QLineEdit>
#include <QClipboard>
#include <QShortcut>
#include <QMessageBox>

#include <TabsPlsCore/FileSystemOp.hpp>
#include <TabsPlsCore/CurrentDirectoryFileOp.hpp>

#include "FileListViewModel.hpp"

static void SetUriListOnClipboard(const QString& data)
{
	auto* clipboard = QApplication::clipboard();
	auto* mimeData = new QMimeData;
	mimeData->setData("text/uri-list", data.toUtf8());
	clipboard->setMimeData(mimeData);
}

FileListTableView::FileListTableView(std::weak_ptr<CurrentDirectoryFileOp> currentDirFileOp):
	m_currentDirFileOp(std::move(currentDirFileOp))
{
	verticalHeader()->hide();
	setShowGrid(false);
	setAcceptDrops(true);

	const auto* copyUrisToClipboardShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C), this);
	connect(copyUrisToClipboardShortcut, &QShortcut::activated, [this]() {return SetUriListOnClipboard(AggregateSelectionDataAsUriList()); });

	const auto pasteUrisFromClipboardShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_V), this);
	connect(pasteUrisFromClipboardShortcut, &QShortcut::activated, [this]() { pasteEvent(); });
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

	mimeData->setData("text/uri-list", AggregateSelectionDataAsUriList().toUtf8());
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
	if (event->mimeData()->hasFormat("text/uri-list")) {
		const auto urls = DecodeFileUris(event->mimeData()->data("text/uri-list"));
		CopyFileUrisIntoCurrentDir(urls);
	}
}

static bool IsValidFileUrl(const QUrl& url)
{
	return url.isValid() && url.scheme() == "file";
}

std::vector<QUrl> FileListTableView::DecodeFileUris(const QString& data)
{
	std::vector<QUrl> uris;
	for (const auto& uri : data.split(QRegExp("[\r\n]"), QString::SkipEmptyParts)) {
		const QUrl url(uri);
		if (!IsValidFileUrl(url))
			continue;
		uris.push_back(url);
	}

	return uris;
}

QString FileListTableView::AggregateSelectionDataAsUriList() const
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

void FileListTableView::pasteEvent()
{
	auto* clipboard = QApplication::clipboard();
	auto* mimeData = clipboard->mimeData();
	if (mimeData->hasFormat("text/uri-list")) {
		const auto urls = DecodeFileUris(mimeData->data("text/uri-list"));
		CopyFileUrisIntoCurrentDir(urls);
	}
}

void FileListTableView::CopyFileUrisIntoCurrentDir(const std::vector<QUrl>& urls)
{
	const auto liveCurrentDirFileOp = m_currentDirFileOp.lock();
	if (!liveCurrentDirFileOp)
		return;

	QStringList failedCopies;

	for (const auto& url : urls) {
		try {
			liveCurrentDirFileOp->CopyRecursive(url.toLocalFile().toStdString(), url.fileName().toStdString());
		}
		catch (const FileSystem::Op::CopyException& e) {
			failedCopies << e.message.c_str();
		}
	}

	if(!failedCopies.empty())
		QMessageBox::warning(this, tr("Problem copying"), failedCopies.join('\n'));

	if (auto* fileListViewModel = dynamic_cast<FileListViewModel*>(model()))
		fileListViewModel->RefreshDirectory(liveCurrentDirFileOp->GetCurrentDir().path().c_str());
}
