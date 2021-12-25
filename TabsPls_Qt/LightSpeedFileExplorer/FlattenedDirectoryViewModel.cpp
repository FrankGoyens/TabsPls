#include "FlattenedDirectoryViewModel.hpp"

#include <QStyle>

#include <TabsPlsCore/FileSystemAlgorithm.hpp>
#include <TabsPlsCore/FileSystemDirectory.hpp>

#include <FileSystemDefsConversion.hpp>

using FileSystem::StringConversion::FromRawPath;
using FileSystem::StringConversion::ToRawPath;

namespace {
const std::vector<QString> tableHeaders = {QObject::tr("Name"), QObject::tr("Size"), QObject::tr("Date modified")};
}

static std::pair<std::vector<FileSystem::FilePath>, std::vector<FileSystem::Directory>>
RetrieveFilesAndDirectories(const FileSystem::Directory& dir) {
    std::vector<FileSystem::FilePath> files;
    std::vector<FileSystem::Directory> dirs;

    for (const auto& entry : FileSystem::GetFilesInDirectory(dir)) {
        if (const auto file = FileSystem::FilePath::FromPath(entry))
            files.push_back(*file);
        else if (const auto dirEntry = FileSystem::Directory::FromPath(entry))
            dirs.push_back(*dirEntry);
    }

    return std::make_pair(files, dirs);
}

namespace {
struct DirectoryReadDispatcher {
    virtual ~DirectoryReadDispatcher() = default;
    virtual void DirectoryReadDispatch(const FileSystem::Directory&) const = 0;
};
} // namespace

static std::vector<FileEntryModel::FileEntry> RetreiveFiles(const FileSystem::Directory& dir,
                                                            const DirectoryReadDispatcher& dispatcher) {
    const auto [files, dirs] = RetrieveFilesAndDirectories(dir);

    for (const auto& childDir : dirs) {
        dispatcher.DirectoryReadDispatch(childDir);
    }
    return FileEntryModel::FilesAsModelEntries(files);
}

static FileSystem::RawPath SubtractBasePath(const FileSystem::RawPath& basePath, const FileSystem::RawPath& filePath) {
    if (basePath.size() > filePath.size())
        return {};

    return FileSystem::Algorithm::StripLeadingPathSeparators({filePath.begin() + basePath.size(), filePath.end()});
}

static FileEntryModel::ModelEntry AsModelEntry(const FileEntryModel::FileEntry& fileEntry,
                                               const FileSystem::RawPath& basePath, const QStyle& styleProvider) {
    const auto fullPath = fileEntry.filePath.path();
    return {FromRawPath(SubtractBasePath(basePath, fullPath)), FileEntryModel::FormatSize(fileEntry.size),
            FileEntryModel::FormatDateLastModified(fileEntry.lastModificationDate), FromRawPath(fullPath),
            styleProvider.standardIcon(QStyle::SP_FileIcon)};
}

static auto AsModelEntries(const std::vector<FileEntryModel::FileEntry>& fileEntries,
                           const FileSystem::RawPath& basePath, QStyle& styleProvider) {
    std::vector<FileEntryModel::ModelEntry> modelEntries;
    std::transform(fileEntries.begin(), fileEntries.end(), std::back_inserter(modelEntries),
                   [&](const auto& fileEntry) { return AsModelEntry(fileEntry, basePath, styleProvider); });
    return modelEntries;
}

namespace {

struct DirectoryReadDispatcherImpl : DirectoryReadDispatcher {
    using ResultReady = std::function<void(std::vector<FileEntryModel::ModelEntry>)>;

    DirectoryReadDispatcherImpl(FileSystem::RawPath basePath, QStyle& styleProvider, ResultReady resultReady)
        : basePath(std::move(basePath)), styleProvider(styleProvider), resultReady(std::move(resultReady)) {}

    void DirectoryReadDispatch(const FileSystem::Directory& dir) const override {
        const auto modelEntries = AsModelEntries(RetreiveFiles(dir, *this), basePath, styleProvider);
        resultReady(modelEntries);
    }

    const FileSystem::RawPath basePath;
    QStyle& styleProvider;

    ResultReady resultReady;
};

} // namespace

FlattenedDirectoryViewModel::FlattenedDirectoryViewModel(QObject* parent, QStyle& styleProvider,
                                                         const QString& initialDirectory)
    : QAbstractTableModel(parent), m_styleProvider(styleProvider) {
    if (const auto dir = FileSystem::Directory::FromPath(ToRawPath(initialDirectory))) {
        StartFileRetreival(*dir);
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

static std::optional<QString> GetDisplayDataForColumn(const std::vector<FileEntryModel::ModelEntry>& entries, int row,
                                                      int column) {
    switch (column) {
    case 0:
        return entries[row].displayName;
    case 1:
        return entries[row].displaySize;
    case 2:
        return entries[row].displayDateModified;
    }
    return {};
}

QVariant FlattenedDirectoryViewModel::data(const QModelIndex& index, int role) const {
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        if (const auto& displayString = GetDisplayDataForColumn(m_modelEntries, index.row(), index.column())) {
            return *displayString;
        }
        return "";
    case Qt::UserRole:
        return m_modelEntries[index.row()].fullPath;
    case Qt::DecorationRole:
        if (index.column() == 0)
            return m_modelEntries[index.row()].icon;
    default:
        break;
    }

    return {};
}

void FlattenedDirectoryViewModel::ChangeDirectory(const QString& newDirectory) {
    if (const auto dir = FileSystem::Directory::FromPath(ToRawPath(newDirectory))) {
        StartFileRetreival(*dir);
    }
}

void FlattenedDirectoryViewModel::RefreshDirectory(const QString& dir) { ChangeDirectory(dir); }

std::optional<std::string> FlattenedDirectoryViewModel::ClaimError() { return {}; }

void FlattenedDirectoryViewModel::StartFileRetreival(const FileSystem::Directory& dir) {
    RetreiveFiles(dir, DirectoryReadDispatcherImpl{dir.path(), m_styleProvider, [this](auto modelEntries) {
                                                       m_modelEntries.insert(m_modelEntries.end(), modelEntries.begin(),
                                                                             modelEntries.end());
                                                   }});
}
