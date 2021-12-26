#include "FlattenedDirectoryViewModel.hpp"

#include <QDebug>
#include <QRunnable>
#include <QStyle>
#include <QThreadPool>

#include <TabsPlsCore/FileSystemAlgorithm.hpp>
#include <TabsPlsCore/FileSystemDirectory.hpp>

#include <FileSystemDefsConversion.hpp>

#include "FileRetrievalByDispatch.hpp"
#include "FileRetrievalRunnable.hpp"

using FileSystem::StringConversion::FromRawPath;
using FileSystem::StringConversion::ToRawPath;

namespace {
const std::vector<QString> tableHeaders = {QObject::tr("Name"), QObject::tr("Size"), QObject::tr("Date modified")};
}

namespace {

struct DirectoryReadDispatcherImpl : FileRetrievalByDispatch::DirectoryReadDispatcher {
    using CreateRunnable = std::function<QRunnable*(const FileSystem::Directory&, const FileSystem::RawPath&)>;

    DirectoryReadDispatcherImpl(FileSystem::RawPath basePath, CreateRunnable createRunnable)
        : basePath(std::move(basePath)), createRunnable(std::move(createRunnable)) {}

    void DirectoryReadDispatch(const FileSystem::Directory& dir) const override {
        if (cancelled)
            return;
        auto* runnable = createRunnable(dir, basePath);
        QThreadPool::globalInstance()->start(runnable);
    }

    bool cancelled = false;
    const FileSystem::RawPath basePath;
    CreateRunnable createRunnable;
};

} // namespace

FlattenedDirectoryViewModel::FlattenedDirectoryViewModel(QObject* parent, QStyle& styleProvider,
                                                         const QString& initialDirectory)
    : QAbstractTableModel(parent), m_modelEntries(&FileEntryModel::ModelEntryDisplayNameSortingPredicate),
      m_styleProvider(styleProvider), m_defaultFileIcon(m_styleProvider.standardIcon(QStyle::SP_FileIcon)) {

    qRegisterMetaType<FileEntryModel::ModelEntry>();
    qRegisterMetaType<FileRetrievalRunnableContainer::NameSortedModelSet>();

    if (const auto dir = FileSystem::Directory::FromPath(ToRawPath(initialDirectory))) {
        ResetDispatcher(*dir);
        StartFileRetrieval(*dir);
    }
}

int FlattenedDirectoryViewModel::rowCount(const QModelIndex&) const { return static_cast<int>(m_modelEntries.size()); }

QVariant FlattenedDirectoryViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (static_cast<unsigned>(section) >= tableHeaders.size())
        return {};

    switch (role) {
    case Qt::DisplayRole:
        return tableHeaders[section];
    default:
        break;
    }

    return {};
}

static std::optional<QString>
GetDisplayDataForColumn(const FileRetrievalRunnableContainer::NameSortedModelSet::const_iterator& entryIt, int column) {
    switch (column) {
    case 0:
        return entryIt->displayName;
    case 1:
        return entryIt->displaySize;
    case 2:
        return entryIt->displayDateModified;
    }
    return {};
}

QVariant FlattenedDirectoryViewModel::data(const QModelIndex& index, int role) const {
    if (index.row() >= m_modelEntries.size())
        return {};

    auto entryIt = m_modelEntries.begin();
    std::advance(entryIt, index.row());

    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        if (const auto& displayString = GetDisplayDataForColumn(entryIt, index.column())) {
            return *displayString;
        }
        return "";
    case Qt::UserRole:
        return entryIt->fullPath;
    case Qt::DecorationRole:
        if (index.column() == 0)
            return entryIt->icon;
    default:
        break;
    }

    return {};
}

void FlattenedDirectoryViewModel::ChangeDirectory(const QString& newDirectory) {
    if (const auto dir = FileSystem::Directory::FromPath(ToRawPath(newDirectory))) {
        beginResetModel();
        m_modelEntries.clear();
        ResetDispatcher(*dir);
        endResetModel();
        StartFileRetrieval(*dir);
    }
}

void FlattenedDirectoryViewModel::RefreshDirectory(const QString& dir) { ChangeDirectory(dir); }

std::optional<std::string> FlattenedDirectoryViewModel::ClaimError() { return {}; }

void FlattenedDirectoryViewModel::StartFileRetrieval(const FileSystem::Directory& dir) {
    if (m_dispatch) {
        m_dispatch->DirectoryReadDispatch(dir);
    }
}

void FlattenedDirectoryViewModel::ResetDispatcher(const FileSystem::Directory& newDirectory) {
    if (auto* dispatchImpl = dynamic_cast<DirectoryReadDispatcherImpl*>(m_dispatch.get()))
        dispatchImpl->cancelled = true;

    m_dispatch = std::make_shared<DirectoryReadDispatcherImpl>(newDirectory.path(), [this](const auto& dir,
                                                                                           const auto& basePath) {
        auto* runnable = new FileRetrievalRunnable(dir, basePath, m_defaultFileIcon, m_dispatch);
        connect(runnable, &FileRetrievalRunnable::resultReady, this, &FlattenedDirectoryViewModel::ReceiveModelEntries);
        return runnable;
    });
}

void FlattenedDirectoryViewModel::ReceiveModelEntries(
    FileRetrievalRunnableContainer::NameSortedModelSet modelEntries,
    const FileRetrievalByDispatch::DirectoryReadDispatcher* usedDispatcher) {
    if (modelEntries.empty() ||
        m_dispatch.get() != usedDispatcher /*residual threads from an old dispatcher could still signal results*/)
        return;

    beginInsertRows(QModelIndex{}, rowCount(), rowCount() + modelEntries.size() - 1);
    m_modelEntries.insert(modelEntries.begin(), modelEntries.end());
    endInsertRows();
}