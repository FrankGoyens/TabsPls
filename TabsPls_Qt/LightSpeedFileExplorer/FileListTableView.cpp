#include "FileListTableView.hpp"

#include <stdexcept>

#include <QApplication>
#include <QClipboard>
#include <QDrag>
#include <QFutureWatcher>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QShortcut>
#include <QTimer>
#include <QtConcurrent/QtConcurrentRun>

#include <TabsPlsCore/CurrentDirectoryFileOp.hpp>
#include <TabsPlsCore/FileSystemOp.hpp>
#include <TabsPlsCore/Send2Trash.hpp>

#include "EscapePodLauncher.hpp"
#include "ExplicitStub.hpp"
#include "FileListViewModel.hpp"
#include "FileSystemDefsConversion.hpp"
#include "FutureWatchDialog.hpp"
#include "QObjectProgressReport.hpp"
#include "QObjectRecycleExceptionHandler.hpp"
#include "ShowIsReadySignaler.hpp"
#include "WindowsNativeContextMenu.hpp"

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

static bool workerThreadBusy = false;

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
            [this]() { PutSelectedItemsIntoClipboardForCutIfAny(); });

    const auto* copyUrisToClipboardShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C), this);
    connect(copyUrisToClipboardShortcut, &QShortcut::activated,
            [this]() { PutSelectedItemsIntoClipboardForCopyIfAny(); });

    const auto* pasteUrisFromClipboardShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_V), this);
    connect(pasteUrisFromClipboardShortcut, &QShortcut::activated, [this]() { pasteEvent(); });

    const auto* recycleItemShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(recycleItemShortcut, &QShortcut::activated, [this]() { AskRecycleSelectedFiles(); });

    const auto* deleteItemShortcut = new QShortcut(Qt::SHIFT + Qt::Key_Delete, this);
    connect(deleteItemShortcut, &QShortcut::activated, [this]() { AskPermanentlyDeleteSelectedFiles(); });

    setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);

    horizontalHeader()->setStretchLastSection(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
}

int FileListTableView::GetModelRoleForFullPaths() { return Qt::UserRole; }

int FileListTableView::GetModelRoleForNames() { return Qt::DisplayRole; }

void FileListTableView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton)
        m_dragStartPosition = event->pos();

    QTableView::mousePressEvent(event);
}

static auto StringifyIntoUriList(const QStringList& list) {
    QStringList dataAsList;
    for (const auto& localFile : list) {
        dataAsList << QUrl::fromLocalFile(localFile).toString();
    }
    return dataAsList.join('\n');
}

void FileListTableView::mouseMoveEvent(QMouseEvent* event) {
    if (!(event->buttons() & Qt::LeftButton)) {
        QTableView::mouseMoveEvent(event);
        return;
    }

    if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance())
        return;

    const auto selectedItemsAsLocalFiles = AggregateSelectionDataAsLocalFileList();
    if (selectedItemsAsLocalFiles.empty())
        return;

    QDrag drag(this);
    QMimeData* mimeData = new QMimeData;

    mimeData->setData("text/uri-list", StringifyIntoUriList(selectedItemsAsLocalFiles).toUtf8());
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

static bool RecyclingIsPossible() { return TabsPlsPython::Send2Trash::ComponentIsAvailable(); }

static void AddAction_Open_IfPossible(QMenu& contextMenu, const std::weak_ptr<CurrentDirectoryFileOp>& currentDirFileOp,
                                      const QStringList& selectedLocalFiles) {
    if (const auto liveCurrentDirFileOp = currentDirFileOp.lock()) {
        if (selectedLocalFiles.size() == 1) {
            const auto uriToOpen = selectedLocalFiles.front();
            auto* contextOpen = contextMenu.addAction(QObject::tr("Open"));
            QObject::connect(contextOpen, &QAction::triggered, [=] {
                EscapePodLauncher::LaunchUrlInWorkingDirectory(QUrl::fromLocalFile(uriToOpen.toUtf8()),
                                                               liveCurrentDirFileOp->GetCurrentDir());
            });
            contextMenu.addSeparator();
        }
    }
}

template <typename RecycleOp>
static void AddAction_Recycle_IfPossible(QMenu& contextMenu, QObject& parent, RecycleOp recycleOp) {
    if (RecyclingIsPossible()) {
        contextMenu.addSeparator();
        auto* contextRecycle = contextMenu.addAction(QObject::tr("Recycle"));
        QObject::connect(contextRecycle, &QAction::triggered, recycleOp);
    }
}

static bool OpenWindowsShellContextMenuIfAvailable(const QPoint& globalPos, const QStringList& selectedItems,
                                                   const std::weak_ptr<CurrentDirectoryFileOp>& currentDirFileOp,
                                                   void* hwnd) {
    try {
        if (WindowsNativeContextMenu::ComponentIsAvailable()) {
            if (selectedItems.empty()) {
                if (const auto liveCurrentDirOp = currentDirFileOp.lock()) {
                    return WindowsNativeContextMenu::ShowContextMenuForItems({liveCurrentDirOp->GetCurrentDir().path()},
                                                                             globalPos.x(), globalPos.y(), hwnd);
                }

            } else {
                std::vector<std::wstring> paths;
                std::transform(selectedItems.begin(), selectedItems.end(), std::back_inserter(paths),
                               [](const auto& selectedItem) { return selectedItem.toStdWString(); });
                return WindowsNativeContextMenu::ShowContextMenuForItems(std::move(paths), globalPos.x(), globalPos.y(),
                                                                         hwnd);
            }
        }
    } catch (const ExplicitStubException&) {
        TabsPlsLog_Debug("Tried to call an explicitly stubbed component: WindowsNativeContextMenu");
    }
    return false;
}

template <typename RenameOp>
static void AddAction_Rename_IfPossible(QMenu& contextMenu, const QModelIndexList& selectedItems, RenameOp renameOp) {
    if (selectedItems.size() == 1) {
        contextMenu.addSeparator();
        auto* contextRename = contextMenu.addAction(QObject::tr("Rename"));
        auto& index = selectedItems.front();
        QObject::connect(contextRename, &QAction::triggered, [=] { renameOp(index); });
    }
}

void FileListTableView::contextMenuEvent(QContextMenuEvent* contextMenuEvent) {
    const auto selectedLocalFiles = AggregateSelectionDataAsLocalFileList();

    if (OpenWindowsShellContextMenuIfAvailable(contextMenuEvent->globalPos(), selectedLocalFiles, m_currentDirFileOp,
                                               (void*)winId()))
        return;

    QMenu contextMenu("Context menu", this);

    const bool hasSelection = !selectedLocalFiles.empty();
    const bool onlyParentDotsAreSelected =
        hasSelection && selectedLocalFiles.size() == 1 && selectedLocalFiles.front() == "..";

    if (hasSelection && !onlyParentDotsAreSelected) {
        AddAction_Open_IfPossible(contextMenu, m_currentDirFileOp, AggregateSelectionDataAsLocalFileList());

        auto* contextCopy = contextMenu.addAction(tr("Copy"));
        connect(contextCopy, &QAction::triggered, [this] { PutSelectedItemsIntoClipboardForCopyIfAny(); });
        auto* contextCut = contextMenu.addAction(tr("Cut"));
        connect(contextCut, &QAction::triggered, [this] { PutSelectedItemsIntoClipboardForCutIfAny(); });
    }

    auto* contextPaste = contextMenu.addAction(tr("Paste"));
    connect(contextPaste, &QAction::triggered, [this] { pasteEvent(); });

    if (hasSelection && !onlyParentDotsAreSelected) {
        AddAction_Rename_IfPossible(contextMenu, selectionModel()->selectedRows(),
                                    [this](const auto& index) { edit(index); });
        AddAction_Recycle_IfPossible(contextMenu, *this, [this] { AskRecycleSelectedFiles(); });
    }

    contextMenu.exec(contextMenuEvent->globalPos());
}

void FileListTableView::ShowCriticalWorkerError(QString title, QString message) {
    QMessageBox::critical(this, title, message);
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

void FileListTableView::NotifyModelOfChange() {
    if (auto* fileListViewModel = dynamic_cast<FileListViewModel*>(model()))
        if (const auto liveCurrentDirFileOp = m_currentDirFileOp.lock())
            fileListViewModel->RefreshDirectory(FromRawPath(liveCurrentDirFileOp->GetCurrentDir().path()));
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

template <typename Watcher>
static std::shared_ptr<QObjectProgressReport> ProvisionWatcherWithProgressReporter(Watcher& watcher) {
    const auto progressReporter = std::make_shared<QObjectProgressReport>();
    watcher.ConnectProgressReporterFromAnotherThread(progressReporter);
    return progressReporter;
}

static ShowIsReadySignaler* InstallReadySignaler(QDialog& watcher) {
    auto* showIsReadySignaler = new ShowIsReadySignaler();
    watcher.installEventFilter(showIsReadySignaler);
    QObject::connect(&watcher, &QObject::destroyed, showIsReadySignaler, &QObject::deleteLater);
    return showIsReadySignaler;
}

void FileListTableView::AskRecycleSelectedFiles() {
    if (workerThreadBusy)
        return;

    if (!TabsPlsPython::Send2Trash::ComponentIsAvailable())
        return AskPermanentlyDeleteSelectedFiles();

    auto entries = AggregateSelectionDataAsLocalFileList();
    if (entries.empty())
        return;

    const auto response = QMessageBox::question(this, tr("Recycle item"), tr("Do you want to recycle these items?"));
    if (response == QMessageBox::StandardButton::Yes) {
        std::vector<std::string> entries_std_string;
        std::transform(entries.begin(), entries.end(), std::back_inserter(entries_std_string),
                       [](const auto& qstring) { return qstring.toStdString(); });

        auto* futureWatchDialog = new FutureWatchDialog(this, tr("Recycle item"));
        auto* const pyThreadState = TabsPlsPython::Send2Trash::BeginThreads();
        workerThreadBusy = true;
        connect(futureWatchDialog, &FutureWatchDialog::accepted, [this, futureWatchDialog, pyThreadState] {
            if (std::holds_alternative<std::shared_ptr<TabsPlsPython::Send2Trash::AggregatedResult>>(
                    futureWatchDialog->Result())) {
                if (const auto result = std::get<std::shared_ptr<TabsPlsPython::Send2Trash::AggregatedResult>>(
                        futureWatchDialog->Result())) {
                    DisplayRecycleFailures(*this, *result);
                }
            }
            workerThreadBusy = false;
            TabsPlsPython::Send2Trash::EndThreads(pyThreadState);
            NotifyModelOfChange();
        });
        connect(futureWatchDialog, &FutureWatchDialog::accepted, futureWatchDialog, &QObject::deleteLater);

        auto* showReadySignaler = InstallReadySignaler(*futureWatchDialog);
        connect(showReadySignaler, &ShowIsReadySignaler::ShowIsReady, [&] {
            auto future = QtConcurrent::run([&]() -> FutureWatchDialog::ResultValue {
                QObjectRecycleExceptionHandler errorHandler;
                ConnectRecyclingErrorSignals(errorHandler);
                return errorHandler
                    .DoWithRecycleExceptionHandling<std::shared_ptr<TabsPlsPython::Send2Trash::AggregatedResult>>(
                        [futureWatchDialog, entries_std_string]() {
                            return std::make_shared<TabsPlsPython::Send2Trash::AggregatedResult>(
                                TabsPlsPython::Send2Trash::SendToTrash(
                                    entries_std_string, ProvisionWatcherWithProgressReporter(*futureWatchDialog)));
                        },
                        std::make_shared<TabsPlsPython::Send2Trash::AggregatedResult>());
            });
            futureWatchDialog->SetFuture(future);
        });

        futureWatchDialog->show();
    }
}

void FileListTableView::ConnectRecyclingErrorSignals(const QObjectRecycleExceptionHandler& recycleErrorHandler) {

    connect(&recycleErrorHandler, &QObjectRecycleExceptionHandler::ModuleNotFound, this,
            &FileListTableView::ShowCriticalWorkerError, Qt::ConnectionType::QueuedConnection);
    connect(&recycleErrorHandler, &QObjectRecycleExceptionHandler::GenericError, this,
            &FileListTableView::ShowCriticalWorkerError, Qt::ConnectionType::QueuedConnection);
    connect(&recycleErrorHandler, &QObjectRecycleExceptionHandler::ExplicitStubError, this,
            &FileListTableView::ShowCriticalWorkerError, Qt::ConnectionType::QueuedConnection);
}

template <typename Data>
static void StartProgressReport(const Data& data, const std::weak_ptr<ProgressReport>& progressReport) {
    if (const auto liveProgressReport = progressReport.lock()) {
        liveProgressReport->SetMinimum(0);
        liveProgressReport->SetMaximum(static_cast<int>(data.size()));
    }
}

static void UpdateProgress(const std::weak_ptr<ProgressReport>& progressReport, int& progress) {
    if (const auto liveProgressReport = progressReport.lock())
        liveProgressReport->UpdateValue(++progress);
}

void FileListTableView::AskPermanentlyDeleteSelectedFiles() {
    if (workerThreadBusy)
        return;

    auto entries = AggregateSelectionDataAsLocalFileList();
    if (entries.empty())
        return;

    const auto response =
        QMessageBox::question(this, tr("Delete file"), tr("Do you want to remove these files? (Cannot be undone!)"));
    if (response == QMessageBox::StandardButton::Yes) {
        auto* futureWatchDialog = new FutureWatchDialog(this, tr("Delete file"));

        workerThreadBusy = true;
        connect(futureWatchDialog, &QDialog::accepted, [this, futureWatchDialog] {
            workerThreadBusy = false;
            if (std::holds_alternative<std::shared_ptr<QStringList>>(futureWatchDialog->Result())) {
                if (const auto result = std::get<std::shared_ptr<QStringList>>(futureWatchDialog->Result())) {
                    CompleteFileOp(tr("Delete file"), *result);
                }
            } else {
                NotifyModelOfChange();
            }
        });
        connect(futureWatchDialog, &QDialog::accepted, futureWatchDialog, &QObject::deleteLater);

        const auto* readySignaler = InstallReadySignaler(*futureWatchDialog);
        connect(readySignaler, &ShowIsReadySignaler::ShowIsReady, [=] {
            const auto future = QtConcurrent::run([entries, futureWatchDialog]() -> FutureWatchDialog::ResultValue {
                const auto progressReport = ProvisionWatcherWithProgressReporter(*futureWatchDialog);
                StartProgressReport(entries, progressReport);

                int progress = 0;

                auto errors = std::make_shared<QStringList>();

                for (const auto& entry : entries) {
                    try {
                        FileSystem::Op::RemoveAll(ToRawPath(entry));
                    } catch (const FileSystem::Op::RemoveAllException& e) {
                        *errors << e.message.c_str();
                    }
                    UpdateProgress(progressReport, progress);
                }
                return errors;
            });
            futureWatchDialog->SetFuture(future);
        });

        futureWatchDialog->show();
    }
}

template <typename Op>
static QStringList PerformOpOnFileUris(const std::vector<QUrl>& urls,
                                       const std::weak_ptr<ProgressReport>& progressReport, const Op& op) {
    QStringList failedCopies;

    StartProgressReport(urls, progressReport);

    int progress = 0;

    for (const auto& url : urls) {
        try {
            op(ToRawPath(url.toLocalFile()), ToRawPath(url.fileName()));
            UpdateProgress(progressReport, progress);
        } catch (const FileSystem::Op::CopyException& e) {
            failedCopies << e.message.c_str();
        } catch (const FileSystem::Op::RenameException& e) {
            failedCopies << e.message.c_str();
        }
    }

    return failedCopies;
}

template <typename OpFunction>
void FileListTableView::DoFileOpWhileShowingProgress(const std::vector<QUrl>& urls, const QString& duringOpTitle,
                                                     const QString& opDoneWithErrortitle, OpFunction opFunction) {
    auto* futureWatcher = new FutureWatchDialog(this, duringOpTitle);

    workerThreadBusy = true;
    connect(futureWatcher, &QDialog::accepted, [this, futureWatcher, opDoneWithErrortitle] {
        if (std::holds_alternative<std::shared_ptr<QStringList>>(futureWatcher->Result())) {
            if (const auto result = std::get<std::shared_ptr<QStringList>>(futureWatcher->Result())) {
                CompleteFileOp(opDoneWithErrortitle, *result);
            }
        }
        workerThreadBusy = false;
    });
    connect(futureWatcher, &QDialog::accepted, futureWatcher, &QObject::deleteLater);

    auto* showReadySignaler = InstallReadySignaler(*futureWatcher);
    connect(showReadySignaler, &ShowIsReadySignaler::ShowIsReady, [&] {
        auto future = QtConcurrent::run([urls, futureWatcher, opFunction]() -> FutureWatchDialog::ResultValue {
            return std::make_shared<QStringList>(
                PerformOpOnFileUris(urls, ProvisionWatcherWithProgressReporter(*futureWatcher), opFunction));
        });
        futureWatcher->SetFuture(future);
    });

    futureWatcher->show();
}

std::optional<std::shared_ptr<CurrentDirectoryFileOp>>
FileListTableView::ValidateFileOpPrecondition(const std::vector<QUrl>& urls) {
    if (workerThreadBusy)
        return {};

    if (urls.empty())
        return {};

    if (const auto liveCurrentDirFileOp = m_currentDirFileOp.lock())
        return liveCurrentDirFileOp;

    return {};
}

void FileListTableView::CopyFileUrisIntoCurrentDir(const std::vector<QUrl>& urls) {
    const auto precondition = ValidateFileOpPrecondition(urls);

    if (!precondition)
        return;

    auto liveCurrentDirFileOp = *precondition;

    DoFileOpWhileShowingProgress(
        urls, tr("Copying"), tr("Problem copying"),
        [liveCurrentDirFileOp](const auto& from, const auto& to) { liveCurrentDirFileOp->CopyRecursive(from, to); });
}

void FileListTableView::MoveFileUrisIntoCurrentDir(const std::vector<QUrl>& urls) {
    const auto precondition = ValidateFileOpPrecondition(urls);

    if (!precondition)
        return;

    auto liveCurrentDirFileOp = *precondition;

    DoFileOpWhileShowingProgress(
        urls, tr("Moving"), tr("Problem moving"),
        [liveCurrentDirFileOp](const auto& from, const auto& to) { liveCurrentDirFileOp->Move(from, to); });
}

void FileListTableView::PutSelectedItemsIntoClipboardForCopyIfAny() {
    const auto uriList = AggregateSelectionDataAsLocalFileList();
    if (uriList.empty())
        return;
    SetUriListOnClipboardForCopy(StringifyIntoUriList(uriList));
}

void FileListTableView::PutSelectedItemsIntoClipboardForCutIfAny() {
    const auto uriList = AggregateSelectionDataAsLocalFileList();
    if (uriList.empty())
        return;
    SetUriListOnClipboardForCut(StringifyIntoUriList(uriList));
}

void FileListTableView::CompleteFileOp(const QString& opName, const QStringList& result) {
    if (!result.empty())
        QMessageBox::warning(this, opName, result.join('\n'));
    NotifyModelOfChange();
}
