#include "FlattenedDirectoryViewModel.hpp"

#include <QDebug>
#include <QRunnable>
#include <QStyle>
#include <QThreadPool>

#include <TabsPlsCore/FileSystemAlgorithm.hpp>
#include <TabsPlsCore/FileSystemDirectory.hpp>

#include <FileSystemDefsConversion.hpp>

#include "AssociatedIconProvider.hpp"
#include "FileRetrievalByDispatch.hpp"
#include "FileRetrievalRunnable.hpp"
#include "IconRetrievalRunnable.hpp"

using FileSystem::StringConversion::FromRawPath;
using FileSystem::StringConversion::ToRawPath;

struct FileRetrievalRunnableProvider {
    virtual ~FileRetrievalRunnableProvider() = default;
    virtual QRunnable* CreateRunnable(const FileSystem::Directory&, const FileSystem::RawPath&, const QIcon&) = 0;
};

namespace {
const std::vector<QString> tableHeaders = {QObject::tr("Name"), QObject::tr("Size"), QObject::tr("Date modified")};

template <typename CreateFunction> struct FileRetrievalRunnableProviderImpl : FileRetrievalRunnableProvider {

    FileRetrievalRunnableProviderImpl(CreateFunction createFunction) : createFunction(std::move(createFunction)) {}

    virtual QRunnable* CreateRunnable(const FileSystem::Directory& dir, const FileSystem::RawPath& basePath,
                                      const QIcon& defaultIcon) override {
        return createFunction(dir, basePath, defaultIcon);
    }

    CreateFunction createFunction;
};

struct DirectoryReadDispatcherImpl : FileRetrievalByDispatch::DirectoryReadDispatcher {

    DirectoryReadDispatcherImpl(FileSystem::RawPath basePath, QIcon defaultIcon,
                                std::weak_ptr<FileRetrievalRunnableProvider> runnableProvider, QThreadPool& threadPool)
        : basePath(std::move(basePath)), defaultIcon(std::move(defaultIcon)),
          runnableProvider(std::move(runnableProvider)), threadPool(threadPool) {}

    void DirectoryReadDispatch(const FileSystem::Directory& dir) const override {
        if (cancelled)
            return;
        if (const auto liveRunnableProvider = runnableProvider.lock()) {
            auto* runnable = liveRunnableProvider->CreateRunnable(dir, basePath, defaultIcon);
            threadPool.start(runnable);
        }
    }

    bool cancelled = false;
    const FileSystem::RawPath basePath;
    QIcon defaultIcon;
    std::weak_ptr<FileRetrievalRunnableProvider> runnableProvider;
    QThreadPool& threadPool;
};

} // namespace

FlattenedDirectoryViewModel::FlattenedDirectoryViewModel(QObject* parent, QStyle& styleProvider,
                                                         const QString& initialDirectory)
    : QAbstractTableModel(parent), m_modelEntries({}, &FileEntryModel::ModelEntryDepthSortingPredicate),
      m_styleProvider(styleProvider), m_defaultFileIcon(m_styleProvider.standardIcon(QStyle::SP_FileIcon)) {

    qRegisterMetaType<FileEntryModel::ModelEntry>();
    qRegisterMetaType<FileRetrievalRunnableContainer::NameSortedModelSet>();

    auto createRunnable = [this](const auto& dir, const auto& basePath, const auto& defaultIcon) {
        auto* runnable = new FileRetrievalRunnable(dir, basePath, defaultIcon, m_dispatch);
        connect(runnable, &FileRetrievalRunnable::resultReady, this, &FlattenedDirectoryViewModel::ReceiveModelEntries);
        return runnable;
    };

    m_runnableProvider =
        std::make_shared<FileRetrievalRunnableProviderImpl<decltype(createRunnable)>>(std::move(createRunnable));

    if (const auto dir = FileSystem::Directory::FromPath(ToRawPath(initialDirectory))) {
        ResetDispatcher(*dir);
        StartFileRetrieval(*dir);
    }
}

FlattenedDirectoryViewModel::~FlattenedDirectoryViewModel() {
    if (const auto liveDispatch = std::dynamic_pointer_cast<DirectoryReadDispatcherImpl>(m_dispatch)) {
        liveDispatch->cancelled = true;
    }

    m_threadPool.clear();
}

int FlattenedDirectoryViewModel::rowCount(const QModelIndex&) const {
    return static_cast<int>(m_modelEntries.get().size());
}

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

static std::optional<QString> GetDisplayDataForColumn(const FileEntryModel::ModelEntry& entry, int column) {
    switch (column) {
    case 0:
        return entry.displayName;
    case 1:
        return entry.displaySize;
    case 2:
        return entry.displayDateModified;
    }
    return {};
}

QVariant FlattenedDirectoryViewModel::data(const QModelIndex& index, int role) const {
    if (index.row() >= m_modelEntries.get().size())
        return {};

    const auto& entry = m_modelEntries.get()[index.row()];

    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        if (const auto& displayString = GetDisplayDataForColumn(entry, index.column())) {
            return *displayString;
        }
        return "";
    case Qt::UserRole:
        return entry.fullPath;
    case Qt::DecorationRole:
        if (index.column() == 0) {
            if (AssociatedIconProvider::ComponentIsAvailable() && entry.icon.name() == m_defaultFileIcon.name()) {
                StartIconRetrieval(entry.fullPath, entry.displayName);
            }

            return entry.icon;
        }
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

void FlattenedDirectoryViewModel::StartIconRetrieval(const QString& fullPath, const QString& displayName) const {
    auto* runnable = new IconRetrievalRunnable(ToRawPath(fullPath), displayName);
    connect(runnable, &IconRetrievalRunnable::resultReady, this, &FlattenedDirectoryViewModel::RefreshIcon);
    m_threadPool.start(runnable);
}

void FlattenedDirectoryViewModel::ResetDispatcher(const FileSystem::Directory& newDirectory) {
    if (auto* dispatchImpl = dynamic_cast<DirectoryReadDispatcherImpl*>(m_dispatch.get()))
        dispatchImpl->cancelled = true;

    m_dispatch = std::make_shared<DirectoryReadDispatcherImpl>(newDirectory.path(), m_defaultFileIcon,
                                                               m_runnableProvider, m_threadPool);
}

void FlattenedDirectoryViewModel::ReceiveModelEntries(
    FileRetrievalRunnableContainer::NameSortedModelSet modelEntries,
    const FileRetrievalByDispatch::DirectoryReadDispatcher* usedDispatcher) {
    if (modelEntries.get().empty() ||
        m_dispatch.get() != usedDispatcher /*residual threads from an old dispatcher could still signal results*/)
        return;

    const auto newEntryAmount = modelEntries.get().size();
    const auto insertIndex = m_modelEntries.lower_bound(modelEntries.get().front()) - m_modelEntries.get().begin();

    beginInsertRows(QModelIndex{}, insertIndex, insertIndex + newEntryAmount - 1);
    m_modelEntries._contiguous_insert(std::move(modelEntries));
    endInsertRows();
}

void FlattenedDirectoryViewModel::RefreshIcon(QIcon icon, const QString&, QVariant reference) {
    if (reference.type() != QVariant::String)
        return;

    const auto displayName = reference.toString();

    const FileEntryModel::ModelEntry dummyEntry{displayName, "", "", "", QIcon{}};
    auto actualEntryIt = m_modelEntries.find(dummyEntry);
    if (actualEntryIt != m_modelEntries.get().end()) {

        // the container is sorted by the display name (which wuold be checked by the static_assert),
        // modifying the icon is safe to do
        static_assert(std::is_same<FileRetrievalRunnableContainer::NameSortedModelSet::key_compare,
                                   decltype(&FileEntryModel::ModelEntryDepthSortingPredicate)>::value);
        auto& mutableModelEntry = const_cast<FileEntryModel::ModelEntry&>(*actualEntryIt);

        mutableModelEntry.icon = icon;
        const auto modelIndex = createIndex(actualEntryIt - m_modelEntries.get().begin(), 0);
        emit dataChanged(modelIndex, modelIndex, {Qt::DecorationRole});
    }
}