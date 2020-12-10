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
#include "FileSystemDefsConversion.hpp"

using FileSystem::StringConversion::ToRawPath;
using FileSystem::StringConversion::FromRawPath;

static QByteArray EncodeWinApiDropEffect(int dropEffect)
{
	QByteArray dropEffectData;
	QDataStream stream(&dropEffectData, QIODevice::WriteOnly);
	stream.setByteOrder(QDataStream::LittleEndian);
	stream << dropEffect;
	return dropEffectData;
}

static int DecodeWinApiDropEffect(QByteArray& bytes)
{
	QDataStream stream(&bytes, QIODevice::ReadOnly);
	stream.setByteOrder(QDataStream::LittleEndian);
	int dropEffect;
	stream >> dropEffect;
	return dropEffect;
}

static void SetDropEffect(QMimeData& mimeData, int dropEffect /* 2 for cut and 5 for copy*/)
{
	mimeData.setData("Preferred DropEffect", EncodeWinApiDropEffect(dropEffect));
	//TODO also add linux support, 
	//on KDE at least the mimetype is application/x-kde-cutselection
	//nautilus is a bit more strange, this stores the following in text/plain (so a mimetype within the mime data):
	//
	//x-special/nautilus-clipboard
	//cut
	//<uris>
}

static void SetUriListOnClipboard(const QString& data, bool copy = true)
{
	auto* clipboard = QApplication::clipboard();
	auto* mimeData = new QMimeData;
	mimeData->setData("text/uri-list", data.toUtf8());
	SetDropEffect(*mimeData, copy ? 5 : 2);

	clipboard->setMimeData(mimeData);
}

static void SetUriListOnClipboardForCopy(const QString& data)
{
	SetUriListOnClipboard(data, true);
}

static void SetUriListOnClipboardForCut(const QString& data)
{
	SetUriListOnClipboard(data, false);
}

FileListTableView::FileListTableView(std::weak_ptr<CurrentDirectoryFileOp> currentDirFileOp):
	m_currentDirFileOp(std::move(currentDirFileOp))
{
	verticalHeader()->hide();
	setShowGrid(false);
	setAcceptDrops(true);

	const auto* cutUrisToClipboardShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_X), this);
	connect(cutUrisToClipboardShortcut, &QShortcut::activated, [this]() {return SetUriListOnClipboardForCut(AggregateSelectionDataAsUriList()); });

	const auto* copyUrisToClipboardShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C), this);
	connect(copyUrisToClipboardShortcut, &QShortcut::activated, [this]() {return SetUriListOnClipboardForCopy(AggregateSelectionDataAsUriList()); });

	const auto* pasteUrisFromClipboardShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_V), this);
	connect(pasteUrisFromClipboardShortcut, &QShortcut::activated, [this]() { pasteEvent(); });

	const auto* deleteItemShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
	connect(deleteItemShortcut, &QShortcut::activated, [this]()
	{
		const auto liveCurrentDirFileOp = m_currentDirFileOp.lock();
		if (!liveCurrentDirFileOp)
			return;

		const auto response = QMessageBox::question(this, tr("Delete file"), tr("Do you want to remove these files? (Cannot be undone!)"));
		if (response == QMessageBox::StandardButton::Yes) {
			for (const auto& entry : AggregateSelectionDataAsLocalFileList())
				FileSystem::Op::RemoveAll(ToRawPath(entry));
			NotifyModelOfChange(*liveCurrentDirFileOp);
		}
	});

	setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);

	horizontalHeader()->setStretchLastSection(true);
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

	drag.exec(Qt::MoveAction);
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

static bool IsValidFileUrl(const QUrl& url)
{
	return url.isValid() && url.scheme() == "file";
}

static std::vector<QUrl> DecodeFileUris(const QString& data)
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

void FileListTableView::dropEvent(QDropEvent* event)
{
	if (event->mimeData()->hasFormat("text/uri-list")) {
		const auto urls = DecodeFileUris(event->mimeData()->data("text/uri-list"));
		CopyFileUrisIntoCurrentDir(urls);
	}
}

void FileListTableView::commitData(QWidget* editor)
{
	QTableView::commitData(editor);

	if(auto* fileListViewModel = dynamic_cast<FileListViewModel*>(model()))
		if(const auto error = fileListViewModel->ClaimError())
			QMessageBox::warning(editor, "Rename", error->c_str());
	
}

QString FileListTableView::AggregateSelectionDataAsUriList() const
{
	const auto selectedLocalFiles = AggregateSelectionDataAsLocalFileList();
	QStringList dataAsList;
	for (const auto& localFile : selectedLocalFiles){
		dataAsList << QUrl::fromLocalFile(localFile).toString();
	}
	return dataAsList.join('\n');
}

QStringList FileListTableView::AggregateSelectionDataAsLocalFileList() const
{
	const auto selectionIndices = selectionModel()->selectedRows();
	QStringList dataAsList;
	for (const auto& index : selectionIndices){
		const QString filePath = model()->data(index, Qt::UserRole).toString();
		if (filePath == "..")
			continue;
		dataAsList << filePath;
	}

	return dataAsList;
}

void FileListTableView::NotifyModelOfChange(CurrentDirectoryFileOp& liveCurrentDirFileOp)
{
	if (auto* fileListViewModel = dynamic_cast<FileListViewModel*>(model()))
		fileListViewModel->RefreshDirectory(FromRawPath(liveCurrentDirFileOp.GetCurrentDir().path()));
}

static std::vector<QUrl> ReadUrlsFromClipboard(const QClipboard& clipboard)
{
	auto* mimeData = clipboard.mimeData();
	if (mimeData->hasFormat("text/uri-list")) {
		return DecodeFileUris(mimeData->data("text/uri-list"));
	}
	return {};
}

void FileListTableView::pasteEvent()
{
	auto* clipboard = QApplication::clipboard();
	auto* mimeData = clipboard->mimeData();
	const auto urls = ReadUrlsFromClipboard(*clipboard);

	if (urls.empty())
		return;

	if (!mimeData->hasFormat("Preferred DropEffect"))
		return CopyFileUrisIntoCurrentDir(urls);

	auto data = mimeData->data("Preferred DropEffect");
	if (DecodeWinApiDropEffect(data) == 2)
		return MoveFileUrisIntoCurrentDir(urls);
	return CopyFileUrisIntoCurrentDir(urls);
}

template<typename Op>
static QStringList PerformOpOnFileUris(const std::vector<QUrl>& urls, const Op& op)
{
	QStringList failedCopies;

	for (const auto& url : urls) {
		try {
			op(ToRawPath(url.toLocalFile()), ToRawPath(url.fileName()));
		}
		catch (const FileSystem::Op::CopyException& e) {
			failedCopies << e.message.c_str();
		}
		catch (const FileSystem::Op::RenameException& e) {
			failedCopies << e.message.c_str();
		}
	}

	return failedCopies;
}

void FileListTableView::CopyFileUrisIntoCurrentDir(const std::vector<QUrl>& urls)
{
	const auto liveCurrentDirFileOp = m_currentDirFileOp.lock();
	if (!liveCurrentDirFileOp)
		return;

	const QStringList failedCopies = PerformOpOnFileUris(urls, 
		[liveCurrentDirFileOp](const auto& from, const auto& to) {liveCurrentDirFileOp->CopyRecursive(from, to); });

	if(!failedCopies.empty())
		QMessageBox::warning(this, tr("Problem copying"), failedCopies.join('\n'));

	NotifyModelOfChange(*liveCurrentDirFileOp);
}

void FileListTableView::MoveFileUrisIntoCurrentDir(const std::vector<QUrl>& urls)
{
	const auto liveCurrentDirFileOp = m_currentDirFileOp.lock();
	if (!liveCurrentDirFileOp)
		return;

	const QStringList failedCopies = PerformOpOnFileUris(urls,
		[liveCurrentDirFileOp](const auto& from, const auto& to) {liveCurrentDirFileOp->Move(from, to); });

	if (!failedCopies.empty())
		QMessageBox::warning(this, tr("Problem moving"), failedCopies.join('\n'));

	NotifyModelOfChange(*liveCurrentDirFileOp);
}
