#include "FileListTableView.hpp"

#include <stdexcept>

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QDrag>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QShortcut>

#include <TabsPlsCore/CurrentDirectoryFileOp.hpp>
#include <TabsPlsCore/FileSystemOp.hpp>
#include <TabsPlsCore/Send2Trash.hpp>

#include "ExplicitStub.hpp"
#include "FileListViewModel.hpp"
#include "FileSystemDefsConversion.hpp"

using FileSystem::StringConversion::FromRawPath;
using FileSystem::StringConversion::ToRawPath;

static QByteArray EncodeWinApiDropEffect(int dropEffect) {
    QByteArray dropEffectData;
    QDataStream stream(&dropEffectData, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << dropEffect;
    return dropEffectData;
}

static int DecodeWinApiDropEffect(QByteArray& bytes) {
    QDataStream stream(&bytes, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    int dropEffect;
    stream >> dropEffect;
    return dropEffect;
}

static constexpr int DROP_EFFECT_CUT = 2;
static constexpr int DROP_EFFECT_COPY = 5;

static void SetDropEffect(QMimeData& mimeData, int dropEffect /* 2 for cut and 5 for copy*/) {
    mimeData.setData("Preferred DropEffect", EncodeWinApiDropEffect(dropEffect));
    // TODO also add linux support,
    // on KDE at least the mimetype is application/x-kde-cutselection
    // nautilus is a bit more strange, this stores the following in text/plain
    // (so a mimetype within the mime data):
    //
    // x-special/nautilus-clipboard
    // cut
    //<uris>
}

static void SetUriListOnClipboard(const QString& data, bool copy = true) {
    auto* clipboard = QApplication::clipboard();
    auto* mimeData = new QMimeData;
    mimeData->setData("text/uri-list", data.toUtf8());
    SetDropEffect(*mimeData, copy ? DROP_EFFECT_COPY : DROP_EFFECT_CUT);

    clipboard->setMimeData(mimeData);
}

static void SetUriListOnClipboardForCopy(const QString& data) { SetUriListOnClipboard(data, true); }

static void SetUriListOnClipboardForCut(const QString& data) { SetUriListOnClipboard(data, false); }

FileListTableView::FileListTableView(std::weak_ptr<CurrentDirectoryFileOp> currentDirFileOp)
    : m_currentDirFileOp(std::move(currentDirFileOp)) {
    verticalHeader()->hide();
    setShowGrid(false);
    setAcceptDrops(true);

    const auto* cutUrisToClipboardShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_X), this);
    connect(cutUrisToClipboardShortcut, &QShortcut::activated,
            [this]() { return SetUriListOnClipboardForCut(AggregateSelectionDataAsUriList()); });

    const auto* copyUrisToClipboardShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C), this);
    connect(copyUrisToClipboardShortcut, &QShortcut::activated,
            [this]() { return SetUriListOnClipboardForCopy(AggregateSelectionDataAsUriList()); });

    const auto* pasteUrisFromClipboardShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_V), this);
    connect(pasteUrisFromClipboardShortcut, &QShortcut::activated, [this]() { pasteEvent(); });

    const auto* recycleItemShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(recycleItemShortcut, &QShortcut::activated, [this]() {
        const auto liveCurrentDirFileOp = m_currentDirFileOp.lock();
        if (!liveCurrentDirFileOp)
            return;

        AskRecycleSelectedFiles(*liveCurrentDirFileOp);
    });

    const auto* deleteItemShortcut = new QShortcut(QKeySequence(Qt::Key_Shift + Qt::Key_Delete), this);
    connect(deleteItemShortcut, &QShortcut::activated, [this]() {
        const auto liveCurrentDirFileOp = m_currentDirFileOp.lock();
        if (!liveCurrentDirFileOp)
            return;

        AskPermanentlyDeleteSelectedFiles(*liveCurrentDirFileOp);
    });

    setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);

    horizontalHeader()->setStretchLastSection(true);
}

int FileListTableView::GetModelRoleForFullPaths() { return Qt::UserRole; }

int FileListTableView::GetModelRoleForNames() { return Qt::DisplayRole; }

void FileListTableView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton)
        m_dragStartPosition = event->pos();

    QTableView::mousePressEvent(event);
}

void FileListTableView::mouseMoveEvent(QMouseEvent* event) {
    if (!(event->buttons() & Qt::LeftButton)) {
        QTableView::mouseMoveEvent(event);
        return;
    }

    if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance())
        return;

    QDrag drag(this);
    QMimeData* mimeData = new QMimeData;

    mimeData->setData("text/uri-list", AggregateSelectionDataAsUriList().toUtf8());
    drag.setMimeData(mimeData);

    drag.exec(Qt::CopyAction | Qt::MoveAction);
}

static void AcceptEventIfFormatIsOK(QDragMoveEvent& event) {
    if (event.mimeData()->hasFormat("text/uri-list")) {
        event.setDropAction(Qt::DropAction::MoveAction);
        event.accept();
    }
}

void FileListTableView::dragEnterEvent(QDragEnterEvent* event) { AcceptEventIfFormatIsOK(*event); }

void FileListTableView::dragMoveEvent(QDragMoveEvent* event) { AcceptEventIfFormatIsOK(*event); }

static bool IsValidFileUrl(const QUrl& url) { return url.isValid() && url.scheme() == "file"; }

static std::vector<QUrl> DecodeFileUris(const QString& data) {
    std::vector<QUrl> uris;
    for (const auto& uri : data.split(QRegExp("[\r\n]"), QString::SkipEmptyParts)) {
        const QUrl url(uri);
        if (!IsValidFileUrl(url))
            continue;
        uris.push_back(url);
    }

    return uris;
}

void FileListTableView::dropEvent(QDropEvent* event) {
    if (event->mimeData()->hasFormat("text/uri-list")) {
        const auto urls = DecodeFileUris(event->mimeData()->data("text/uri-list"));
        MoveFileUrisIntoCurrentDir(urls);
    }
}

void FileListTableView::commitData(QWidget* editor) {
    QTableView::commitData(editor);

    if (auto* fileListViewModel = dynamic_cast<FileListViewModel*>(model()))
        if (const auto error = fileListViewModel->ClaimError())
            QMessageBox::warning(editor, "Rename", error->c_str());
}

QString FileListTableView::AggregateSelectionDataAsUriList() const {
    const auto selectedLocalFiles = AggregateSelectionDataAsLocalFileList();
    QStringList dataAsList;
    for (const auto& localFile : selectedLocalFiles) {
        dataAsList << QUrl::fromLocalFile(localFile).toString();
    }
    return dataAsList.join('\n');
}

QStringList FileListTableView::AggregateSelectionDataAsLocalFileList() const {
    const auto selectionIndices = selectionModel()->selectedRows();
    QStringList dataAsList;
    for (const auto& index : selectionIndices) {
        const QString filePath = model()->data(index, Qt::UserRole).toString();
        if (filePath == "..")
            continue;
        dataAsList << filePath;
    }

    return dataAsList;
}

void FileListTableView::NotifyModelOfChange(CurrentDirectoryFileOp& liveCurrentDirFileOp) {
    if (auto* fileListViewModel = dynamic_cast<FileListViewModel*>(model()))
        fileListViewModel->RefreshDirectory(FromRawPath(liveCurrentDirFileOp.GetCurrentDir().path()));
}

static std::vector<QUrl> ReadUrlsFromClipboard(const QClipboard& clipboard) {
    auto* mimeData = clipboard.mimeData();
    if (mimeData->hasFormat("text/uri-list")) {
        return DecodeFileUris(mimeData->data("text/uri-list"));
    }
    return {};
}

void FileListTableView::pasteEvent() {
    auto* clipboard = QApplication::clipboard();
    auto* mimeData = clipboard->mimeData();
    const auto urls = ReadUrlsFromClipboard(*clipboard);

    PerformMimeDataActionOnIncomingFiles(*mimeData, urls);
}

void FileListTableView::PerformMimeDataActionOnIncomingFiles(const QMimeData& mimeData, const std::vector<QUrl>& urls) {
    if (urls.empty())
        return;

    if (!mimeData.hasFormat("Preferred DropEffect"))
        return CopyFileUrisIntoCurrentDir(urls);

    auto data = mimeData.data("Preferred DropEffect");
    if (DecodeWinApiDropEffect(data) == DROP_EFFECT_CUT)
        return MoveFileUrisIntoCurrentDir(urls);
    return CopyFileUrisIntoCurrentDir(urls);
}

static void DisplayRecycleFailures(QWidget& parent, TabsPlsPython::Send2Trash::AggregatedResult result) {
    result.itemResults.erase(std::remove_if(result.itemResults.begin(), result.itemResults.end(),
                                            [](const auto& failedItem) { return !failedItem.second.error; }),
                             result.itemResults.end());

    std::vector<QString> errors;
    std::transform(result.itemResults.begin(), result.itemResults.end(), std::back_inserter(errors),
                   [](const auto& errorItem) {
                       return QString::fromStdString(errorItem.first + std::string(": ") + *errorItem.second.error);
                   });

    if (!errors.empty()) {
        QString warningMessage = QObject::tr("The following items could not be recycled:\n");
        for (auto it = errors.begin(); it != errors.end() - 1; ++it) {
            warningMessage += *it + "\n";
        }
        warningMessage += errors.back();
        QMessageBox::warning(&parent, QObject::tr("Recycle item"), warningMessage);
    }
}

template <typename F> static auto DoWithRecycleExceptionHandling(QWidget& parent, F func) {
    try {
        return func();
    } catch (const TabsPlsPython::Send2Trash::ModuleNotFoundException&) {
        QMessageBox::critical(&parent, QObject::tr("Recycle item"),
                              QObject::tr("The send2trash module could not be found, ") +
                                  QObject::tr("please reinstall this program."));
    } catch (const TabsPlsPython::Send2Trash::Exception&) {
        QMessageBox::critical(&parent, QObject::tr("Recycle item"), QObject::tr("Unknown eror"));
    } catch (const ExplicitStubException&) {
        // The component is supposedly available, but we're still somehow calling the stubbed implementation. But
        // let's not bother the user with this information
        QMessageBox::critical(&parent, QObject::tr("Recycle item"), QObject::tr("Unknown error"));
    }
}

void FileListTableView::AskRecycleSelectedFiles(CurrentDirectoryFileOp& liveCurrentDirFileOp) {
    if (!TabsPlsPython::Send2Trash::ComponentIsAvailable())
        return AskPermanentlyDeleteSelectedFiles(liveCurrentDirFileOp);

    auto entries = AggregateSelectionDataAsLocalFileList();
    if (entries.empty())
        return;

    const auto response = QMessageBox::question(this, tr("Recycle item"), tr("Do you want to recycle these items?"));
    if (response == QMessageBox::StandardButton::Yes) {
        std::vector<std::string> entries_std_string;
        std::transform(entries.begin(), entries.end(), std::back_inserter(entries_std_string),
                       [](const auto& qstring) { return qstring.toStdString(); });

        DoWithRecycleExceptionHandling(*this, [&]() {
            auto result = TabsPlsPython::Send2Trash::SendToTrash(entries_std_string);
            DisplayRecycleFailures(*this, result);
        });

        NotifyModelOfChange(liveCurrentDirFileOp);
    }
}

void FileListTableView::AskPermanentlyDeleteSelectedFiles(CurrentDirectoryFileOp& liveCurrentDirFileOp) {
    auto entries = AggregateSelectionDataAsLocalFileList();
    if (entries.empty())
        return;

    const auto response =
        QMessageBox::question(this, tr("Delete file"), tr("Do you want to remove these files? (Cannot be undone!)"));
    if (response == QMessageBox::StandardButton::Yes) {
        for (const auto& entry : entries)
            FileSystem::Op::RemoveAll(ToRawPath(entry));
        NotifyModelOfChange(liveCurrentDirFileOp);
    }
}

template <typename Op> static QStringList PerformOpOnFileUris(const std::vector<QUrl>& urls, const Op& op) {
    QStringList failedCopies;

    for (const auto& url : urls) {
        try {
            op(ToRawPath(url.toLocalFile()), ToRawPath(url.fileName()));
        } catch (const FileSystem::Op::CopyException& e) {
            failedCopies << e.message.c_str();
        } catch (const FileSystem::Op::RenameException& e) {
            failedCopies << e.message.c_str();
        }
    }

    return failedCopies;
}

void FileListTableView::CopyFileUrisIntoCurrentDir(const std::vector<QUrl>& urls) {
    const auto liveCurrentDirFileOp = m_currentDirFileOp.lock();
    if (!liveCurrentDirFileOp)
        return;

    const QStringList failedCopies =
        PerformOpOnFileUris(urls, [liveCurrentDirFileOp](const auto& from, const auto& to) {
            liveCurrentDirFileOp->CopyRecursive(from, to);
        });

    if (!failedCopies.empty())
        QMessageBox::warning(this, tr("Problem copying"), failedCopies.join('\n'));

    NotifyModelOfChange(*liveCurrentDirFileOp);
}

void FileListTableView::MoveFileUrisIntoCurrentDir(const std::vector<QUrl>& urls) {
    const auto liveCurrentDirFileOp = m_currentDirFileOp.lock();
    if (!liveCurrentDirFileOp)
        return;

    const QStringList failedCopies = PerformOpOnFileUris(
        urls, [liveCurrentDirFileOp](const auto& from, const auto& to) { liveCurrentDirFileOp->Move(from, to); });

    if (!failedCopies.empty())
        QMessageBox::warning(this, tr("Problem moving"), failedCopies.join('\n'));

    NotifyModelOfChange(*liveCurrentDirFileOp);
}
