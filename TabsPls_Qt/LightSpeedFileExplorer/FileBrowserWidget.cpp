#include "FileBrowserWidget.hpp"

#include <filesystem>

#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QSizePolicy>
#include <QUrl>
#include <QVBoxLayout>

#include <TabsPlsCore/FileSystemOp.hpp>

#include "CurrentDirectoryFileOpQtImpl.hpp"
#include "DirectoryInputField.hpp"
#include "EscapePodLauncher.hpp"
#include "FileBrowserViewModelProvider.hpp"
#include "FileListTableView.hpp"
#include "FileListTableViewWithFilter.hpp"
#include "FileListViewModel.hpp"
#include "FileSystemDefsConversion.hpp"
#include "FlattenedDirectoryViewModel.hpp"

using FileSystem::StringConversion::FromName;
using FileSystem::StringConversion::FromRawPath;
using FileSystem::StringConversion::ToRawPath;

namespace {
using CurrentDirSetter = std::function<void(FileSystem::Directory)>;

struct DirectoryChangeException : std::exception {
    DirectoryChangeException(const char* message_) : message(message_) {}

    const char* what() const noexcept override { return message.c_str(); }

    std::string message;
};
} // namespace

/*The FileListViewModel doesn't know if there is something wrong before clearing
 * its data, so when something does go wrong it needs a refresh*/
template <typename Func>
static auto DoFuncWithModelRefreshWhenExceptionIsThrown(Func f,
                                                        std::weak_ptr<FileBrowserViewModelProvider> modelProvider,
                                                        const FileSystem::Directory& currentDir) {
    try {
        return f();
    } catch (const DirectoryChangeException&) {
        if (const auto liveModelProvider = modelProvider.lock()) {
            if (auto* directoryChanger = liveModelProvider->GetDirectoryChangerForActiveModel())
                directoryChanger->RefreshDirectory(FromRawPath(currentDir.path()));
        }
        throw;
    }
}

static DirectoryChanger* GetDirectoryChanger(const FileListTableViewWithFilter& fileListViewWithFilter) {
    if (const auto modelProvider = fileListViewWithFilter.GetModelProvider().lock()) {
        if (auto* directoryChanger = modelProvider->GetDirectoryChangerForActiveModel()) {
            return directoryChanger;
        }
    }
    return nullptr;
}

static DirectoryChanger* GetDirectoryChanger(std::weak_ptr<const FileBrowserViewModelProvider> modelProviderPtr) {
    if (const auto liveModelProvider = modelProviderPtr.lock()) {
        if (auto* directoryChanger = liveModelProvider->GetDirectoryChangerForActiveModel()) {
            return directoryChanger;
        }
    }
    return nullptr;
}

static QAbstractTableModel* GetModel(std::weak_ptr<const FileBrowserViewModelProvider> modelProviderPtr) {
    if (const auto liveModelProvider = modelProviderPtr.lock()) {
        if (auto* liveModel = liveModelProvider->GetActiveModel()) {
            return liveModel;
        }
    }
    return nullptr;
}

static void ChangeDirectoryWithoutHistoryUpdate(const FileSystem::Directory& newDir,
                                                FileListTableViewWithFilter& fileListViewWithFilter,
                                                QLineEdit& directoryInputField, CurrentDirSetter currentDirSetter) {
    if (auto* directoryChanger = GetDirectoryChanger(fileListViewWithFilter)) {
        const auto newDirString = FromRawPath(newDir.path());
        directoryChanger->ChangeDirectory(newDirString);
        if (const auto error = directoryChanger->ClaimError())
            throw DirectoryChangeException(error->c_str());
        directoryInputField.setText(newDirString);
        fileListViewWithFilter.ClearFilter();
        currentDirSetter(newDir);
    }
}

static auto CreateDirectoryChangedByGoingToParent(RobustDirectoryHistoryStore& history,
                                                  FileListTableViewWithFilter& fileListViewWithFilter,
                                                  QLineEdit& directoryInputField, CurrentDirSetter currentDirSetter) {
    return [&, currentDirSetter]() {
        try {
            const auto newDir = history.GetCurrent().Parent();
            if (newDir.path() == history.GetCurrent().path())
                return;
            auto historyTemp = history;
            historyTemp.OnNewDirectory(newDir);

            DoFuncWithModelRefreshWhenExceptionIsThrown(
                [&, currentDirSetter]() {
                    ChangeDirectoryWithoutHistoryUpdate(newDir, fileListViewWithFilter, directoryInputField,
                                                        currentDirSetter);
                },
                fileListViewWithFilter.GetModelProvider(), history.GetCurrent());

            history = std::move(historyTemp);
        } catch (const StoreIsEmptyException&) {
        }
    };
}

static auto CreateDirectoryChangedClosure(RobustDirectoryHistoryStore& history,
                                          FileListTableViewWithFilter& fileListViewWithFilter,
                                          QLineEdit& directoryInputField, CurrentDirSetter currentDirSetter) {
    return [&, currentDirSetter](const FileSystem::Directory& newDir) {
        auto historyTemp = history;
        historyTemp.OnNewDirectory(newDir);
        DoFuncWithModelRefreshWhenExceptionIsThrown(
            [&, currentDirSetter]() {
                ChangeDirectoryWithoutHistoryUpdate(newDir, fileListViewWithFilter, directoryInputField,
                                                    currentDirSetter);
            },
            fileListViewWithFilter.GetModelProvider(), history.GetCurrent());
        history = std::move(historyTemp);
    };
}

namespace {
struct HistoryBackVariant {};
struct HistoryForwardVariant {};
} // namespace

static void SwitchHistory(RobustDirectoryHistoryStore& history, const HistoryBackVariant&) {
    history.SwitchToPrevious();
}

static void SwitchHistory(RobustDirectoryHistoryStore& history, const HistoryForwardVariant&) {
    history.SwitchToNext();
}

template <typename HistoryVariant>
static auto CreateHistoryActionClosure(RobustDirectoryHistoryStore& history,
                                       FileListTableViewWithFilter& fileListViewWithFilter,
                                       QLineEdit& directoryInputField, CurrentDirSetter currentDirSetter,
                                       const HistoryVariant& variant) {
    return [&, currentDirSetter]() {
        try {
            auto historyTemp = history;
            SwitchHistory(historyTemp, variant);

            DoFuncWithModelRefreshWhenExceptionIsThrown(
                [&, currentDirSetter]() {
                    ChangeDirectoryWithoutHistoryUpdate(historyTemp.GetCurrent(), fileListViewWithFilter,
                                                        directoryInputField, currentDirSetter);
                },
                fileListViewWithFilter.GetModelProvider(), history.GetCurrent());

            history = std::move(historyTemp);
        } catch (const ImpossibleSwitchException&) {
        }
    };
}

static auto* CreateNewDirectoryButton(const QStyle& styleProvider) {
    auto* newDirectoryButton = new QPushButton("+");
    newDirectoryButton->setIcon(styleProvider.standardIcon(QStyle::SP_DirIcon));
    return newDirectoryButton;
}

static auto SetupTopBar(QWidget& widget, const QString& initialDirectory) {
    auto* vbox = new QHBoxLayout();
    auto* backButton = new QPushButton(u8"←");
    auto* forwardButton = new QPushButton(u8"→");
    auto* directoryInputField = new DirectoryInputField(initialDirectory);
    auto* newDirectoryButton = CreateNewDirectoryButton(*widget.style());

    backButton->setFixedWidth(50);
    forwardButton->setFixedWidth(50);
    newDirectoryButton->setFixedWidth(50);

    vbox->addWidget(backButton);
    vbox->addWidget(forwardButton);
    vbox->addWidget(directoryInputField);
    vbox->addWidget(newDirectoryButton);
    vbox->setMargin(0);

    widget.setLayout(vbox);

    return std::make_tuple(backButton, forwardButton, directoryInputField, newDirectoryButton);
}

static auto SetupCentralWidget(QWidget& fileBrowserWidget, std::shared_ptr<CurrentDirectoryFileOp> currentDirFileOp,
                               std::unique_ptr<QAbstractTableModel> viewModel, const QString& initialDirectory) {
    auto* topBarWidget = new QWidget();
    auto [backButton, forwardButton, topBarDirectoryInputField, newDirectoryButton] =
        SetupTopBar(*topBarWidget, initialDirectory);

    auto* fileListViewWidget = new FileListTableViewWithFilter(currentDirFileOp, std::move(viewModel));

    fileListViewWidget->GetFileListTableView().resizeColumnsToContents();
    QObject::connect(fileListViewWidget->GetModelProvider().lock().get(), &FileBrowserViewModelProvider::modelReset,
                     [fileListViewWidget] { fileListViewWidget->GetFileListTableView().resizeColumnsToContents(); });
    QObject::connect(fileListViewWidget->GetModelProvider().lock().get(), &FileBrowserViewModelProvider::rowsInserted,
                     [fileListViewWidget](const QModelIndex&, int first, int) {
                         if (first < 2)
                             // Resize for the first two batches of inserted rows,
                             // because resizing the columns is expensive, and the view starts at the top
                             fileListViewWidget->GetFileListTableView().resizeColumnsToContents();
                     });

    auto* fileListViewActiveFilterLabel = new QLabel();

    auto* rootLayout = new QVBoxLayout(&fileBrowserWidget);
    rootLayout->addWidget(topBarWidget);
    rootLayout->addWidget(fileListViewWidget);
    rootLayout->addWidget(fileListViewActiveFilterLabel);

    fileBrowserWidget.setLayout(rootLayout);

    return std::make_tuple(fileListViewWidget, fileListViewWidget->GetModelProvider(), topBarDirectoryInputField,
                           backButton, forwardButton, newDirectoryButton);
}

FileBrowserWidget::FileBrowserWidget(FileSystem::Directory initialDir) : m_currentDirectory(std::move(initialDir)) {
    m_historyStore.OnNewDirectory(m_currentDirectory);

    m_currentDirFileOpImpl = std::make_shared<CurrentDirectoryFileOpQtImpl>(m_currentDirectory);

    const auto [fileListViewWidget_from_binding, fileListViewModelProvider_from_binding,
                topBarDirectoryInputField_from_binding, backButton, forwardButton, newDirectoryButton] =
        SetupCentralWidget(*this, m_currentDirFileOpImpl,
                           std::make_unique<FileListViewModel>(this, *style(), FromRawPath(m_currentDirectory.path())),
                           FromRawPath(m_currentDirectory.path()));

    m_fileListTableView = fileListViewWidget_from_binding;
    auto fileListViewModelProvider = fileListViewModelProvider_from_binding; // This is done to be able to capture
                                                                             // this variable in lambda's
    auto* topBarDirectoryInputField = topBarDirectoryInputField_from_binding;

    const auto setCurrentDirectoryMemberCall = [this](FileSystem::Directory newDir) {
        SetCurrentDirectory(std::move(newDir));
    };

    const auto directoryChangedClosure = CreateDirectoryChangedClosure(
        m_historyStore, *m_fileListTableView, *topBarDirectoryInputField, setCurrentDirectoryMemberCall);

    connect(topBarDirectoryInputField, &DirectoryInputField::directoryChanged, [=](const auto& dirString) {
        if (const auto dir = FileSystem::Directory::FromPath(ToRawPath(dirString)))
            DisplayDirectoryChangedErrorIfExceptionHappens([&]() { directoryChangedClosure(*dir); });
    });

    connect(topBarDirectoryInputField, &DirectoryInputField::directoryChanged,
            &m_fileListTableView->GetFileListTableView(), qOverload<>(&QWidget::setFocus));

    const auto backActionClosure =
        CreateHistoryActionClosure(m_historyStore, *m_fileListTableView, *topBarDirectoryInputField,
                                   setCurrentDirectoryMemberCall, HistoryBackVariant());

    const auto forwardActionClosure =
        CreateHistoryActionClosure(m_historyStore, *m_fileListTableView, *topBarDirectoryInputField,
                                   setCurrentDirectoryMemberCall, HistoryForwardVariant());

    backButton->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Left));
    forwardButton->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Right));

    connect(backButton, &QPushButton::pressed,
            [this, backActionClosure]() { DisplayDirectoryChangedErrorIfExceptionHappens(backActionClosure); });
    connect(forwardButton, &QPushButton::pressed,
            [this, forwardActionClosure]() { DisplayDirectoryChangedErrorIfExceptionHappens(forwardActionClosure); });

    const auto directoryChangedByGoingToParentClosure = CreateDirectoryChangedByGoingToParent(
        m_historyStore, *m_fileListTableView, *topBarDirectoryInputField, setCurrentDirectoryMemberCall);

    connect(&m_fileListTableView->GetFileListTableView(), &QTableView::activated, [=](const QModelIndex& index) {
        if (auto* liveModel = GetModel(fileListViewModelProvider)) {
            const auto itemString =
                liveModel->data(index, m_fileListTableView->GetFileListTableView().GetModelRoleForFullPaths());

            if (itemString == "..")
                DisplayDirectoryChangedErrorIfExceptionHappens([&]() { directoryChangedByGoingToParentClosure(); });
            else if (const auto dir = FileSystem::Directory::FromPath(ToRawPath(itemString.toString())))
                DisplayDirectoryChangedErrorIfExceptionHappens([&]() { directoryChangedClosure(*dir); });
            else if (const auto file = FileSystem::FilePath::FromPath(ToRawPath(itemString.toString()))) {
                EscapePodLauncher::LaunchUrlInWorkingDirectory(QUrl::fromLocalFile(itemString.toString()),
                                                               FileSystem::Directory::FromFilePathParent(*file));
            }
        }
    });

    const auto* gotoParentActionShortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Up), this);
    connect(gotoParentActionShortcut, &QShortcut::activated, [this, directoryChangedByGoingToParentClosure]() {
        DisplayDirectoryChangedErrorIfExceptionHappens(directoryChangedByGoingToParentClosure);
    });

    const auto* focusAndSelectDirectoryInputField = new QShortcut(QKeySequence(Qt::Key_F6), this);
    connect(focusAndSelectDirectoryInputField, &QShortcut::activated, [=]() {
        topBarDirectoryInputField->setFocus();
        topBarDirectoryInputField->selectAll();
    });

    connect(newDirectoryButton, &QPushButton::pressed, [=]() {
        bool ok = false;
        const auto nameForNewDir = QInputDialog::getText(this, tr("New Folder"), tr("New folder name"),
                                                         QLineEdit::Normal, tr("New Folder"), &ok);
        if (ok) {
            try {
                FileSystem::Op::CreateDirectory(m_currentDirectory, ToRawPath(nameForNewDir));
                if (auto* directoryChanger = GetDirectoryChanger(fileListViewModelProvider)) {
                    directoryChanger->RefreshDirectory(FromRawPath(m_currentDirectory.path()));
                }
            } catch (const FileSystem::Op::CreateDirectoryException& e) {
                QMessageBox::warning(this, tr("Create new folder failed"), e.what());
            }
        }
    });

    connect(&m_fs_watcher, &QFileSystemWatcher::directoryChanged, [=](const QString&) {
        if (auto* directoryChanger = GetDirectoryChanger(fileListViewModelProvider)) {
            directoryChanger->RefreshDirectory(FromRawPath(m_currentDirectory.path()));
        }
    });

    StartWatchingCurrentDirectory();
}

const QString FileBrowserWidget::GetCurrentDirectoryName() const {
    return FromName(FileSystem::GetDirectoryname(m_currentDirectory));
}

void FileBrowserWidget::RequestChangeToFlatDirectoryStructure() { m_fileListTableView->RequestFlatModel(); }

void FileBrowserWidget::RequestChangeToHierarchyDirectoryStructure() { m_fileListTableView->RequestHierarchyModel(); }

void FileBrowserWidget::SetCurrentDirectory(FileSystem::Directory newDir) {
    StopWatchingCurrentDirectory();

    m_currentDirectory = std::move(newDir);
    StartWatchingCurrentDirectory();

    m_currentDirFileOpImpl->updateCurrentDir(m_currentDirectory);
    emit currentDirectoryNameChanged(GetCurrentDirectoryName());
}

void FileBrowserWidget::StartWatchingCurrentDirectory() {
    m_fs_watcher.addPath(FromRawPath(m_currentDirectory.path()));
}

void FileBrowserWidget::StopWatchingCurrentDirectory() {
    m_fs_watcher.removePath(FromRawPath(m_currentDirectory.path()));
}

template <typename Func> inline void FileBrowserWidget::DisplayDirectoryChangedErrorIfExceptionHappens(Func f) {
    try {
        f();
    } catch (const DirectoryChangeException& e) {
        DisplayDirectoryChangedError(e.what());
    } catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Unknown error"), e.what());
    }
}

void FileBrowserWidget::DisplayDirectoryChangedError(const char* message) {
    QMessageBox::critical(this, tr("Unable to change directory"), message);
}
