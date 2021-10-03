#include "FileListViewModel.hpp"

#include <thread>

#include <QStyle>
#include <QThreadPool>

#include <TabsPlsCore/FileSystem.hpp>
#include <TabsPlsCore/FileSystemAlgorithm.hpp>
#include <TabsPlsCore/FileSystemDirectory.hpp>
#include <TabsPlsCore/FileSystemOp.hpp>

#include "AssociatedIconProvider.hpp"
#include "FileSystemDefsConversion.hpp"
#include "IconRetrievalRunnable.hpp"

using FileSystem::StringConversion::FromRawPath;
using FileSystem::StringConversion::ToRawPath;

namespace {
const std::vector<QString> tableHeaders = {QObject::tr("Name"), QObject::tr("Size"), QObject::tr("Date modified")};

struct GetFilesInDirectoryException : std::exception {
    GetFilesInDirectoryException(const char* message_) : message(message_) {}

    const char* what() const noexcept override { return message.c_str(); }

    std::string message;
};
} // namespace

static auto GetSizesForFiles(const std::vector<FileSystem::FilePath>& files) {
    std::vector<FileListViewModel::FileEntry> filesWithSizes;
    std::transform(files.begin(), files.end(), std::back_inserter(filesWithSizes), [](const auto& file) {
        return FileListViewModel::FileEntry{file, FileSystem::GetLastWriteTime(file), FileSystem::GetFileSize(file)};
    });
    return filesWithSizes;
}

static std::pair<std::vector<FileListViewModel::FileEntry>, std::vector<FileSystem::Directory>>
GetFilesInDirectoryWithFSCatch(const FileSystem::Directory& dir) {
    try {
        std::vector<FileSystem::FilePath> files;
        std::vector<FileSystem::Directory> dirs;

        for (const auto& entry : FileSystem::GetFilesInDirectory(dir)) {
            if (auto file = FileSystem::FilePath::FromPath(entry)) {
                files.emplace_back(*file);
                continue;
            }

            if (auto dir = FileSystem::Directory::FromPath(entry))
                dirs.emplace_back(*dir);
        }

        return std::make_pair(GetSizesForFiles(files), dirs);
    } catch (const std::exception& e) {
        throw GetFilesInDirectoryException(e.what());
    }

    return {{}, {}};
}

static std::pair<std::vector<FileListViewModel::FileEntry>, std::vector<FileSystem::Directory>>
RetrieveDirectoryContents(const QString& initialDirectory) {
    if (auto dir = FileSystem::Directory::FromPath(ToRawPath(initialDirectory)))
        return GetFilesInDirectoryWithFSCatch(*dir);

    return {{}, {}};
}

FileListViewModel::FileListViewModel(QObject* parent, QStyle& styleProvider, const QString& initialDirectory)
    : QAbstractTableModel(parent), m_styleProvider(styleProvider) {
    std::tie(m_fileEntries, m_directoryEntries) = RetrieveDirectoryContents(initialDirectory);
    FillModelDataCheckingForRoot(initialDirectory);
}

QVariant FileListViewModel::data(const QModelIndex& index, int role) const {
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        if (const auto& displayColumn = GetDisplayDataForColumn(index.column())) {
            return displayColumn->get()[index.row()];
        }
        return "";
    case Qt::UserRole:
        return m_fullPaths[index.row()];
    case Qt::DecorationRole:
        if (index.column() == 0)
            return m_icons[index.row()];
    default:
        break;
    }

    return {};
}

std::optional<std::reference_wrapper<const std::vector<QString>>>
FileListViewModel::GetDisplayDataForColumn(int column) const {
    switch (column) {
    case 0:
        return m_displayName;
    case 1:
        return m_displaySize;
    case 2:
        return m_displayDateModified;
    }
    return {};
}

bool FileListViewModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (value.type() == QVariant::Type::String) {

        const auto fullPathAtIndex = ToRawPath(data(index, Qt::UserRole).toString());

        std::optional<std::pair<FileSystem::RawPath, FileSystem::RawPath>> renameCall = {};
        FileSystem::RawPath parentPath;

        if (const auto dir = FileSystem::Directory::FromPath(fullPathAtIndex)) {
            parentPath = FileSystem::Algorithm::StripTrailingPathSeparators(dir->Parent().path());
            if (FileSystem::Algorithm::StripTrailingPathSeparators(dir->path()) == parentPath)
                return false;

            renameCall = std::make_pair(
                dir->path(), parentPath + FileSystem::Separator() +
                                 FileSystem::Algorithm::StripLeadingPathSeparators(ToRawPath(value.toString())));
        } else if (const auto file = FileSystem::FilePath::FromPath(fullPathAtIndex)) {
            parentPath = FileSystem::Algorithm::StripTrailingPathSeparators(
                FileSystem::Directory::FromFilePathParent(*file).path());
            renameCall = std::make_pair(
                file->path(), parentPath + FileSystem::Separator() +
                                  FileSystem::Algorithm::StripLeadingPathSeparators(ToRawPath(value.toString())));
        }

        if (!renameCall)
            return false;

        try {
            FileSystem::Op::Rename(renameCall->first, renameCall->second);
            RefreshDirectory(FromRawPath(parentPath));
        } catch (const FileSystem::Op::RenameException& e) {
            m_error = e.message;
            return false;
        }

        return true;
    } else if (role == Qt::DecorationRole && value.type() == QVariant::Type::Icon) {
        m_icons[index.row()] = qvariant_cast<QIcon>(value);
        return true;
    }
    return false;
}

Qt::ItemFlags FileListViewModel::flags(const QModelIndex& index) const {
    const auto defaultFlags = QAbstractTableModel::flags(index);

    if (m_displayName.empty())
        return defaultFlags;

    // The parent directory field should be readonly
    if (m_displayName.front() == ".." && index.row() == 0)
        return defaultFlags;

    // Anything other than the name field should be readonly
    if (index.column() > 0)
        return defaultFlags;

    return QAbstractTableModel::flags(index) | Qt::ItemFlag::ItemIsEditable;
}

void FileListViewModel::ChangeDirectory(const QString& dir) {
    beginResetModel();
    try {
        std::tie(m_fileEntries, m_directoryEntries) = RetrieveDirectoryContents(dir);
        m_displaySize.clear();
        m_displayDateModified.clear();
    } catch (const GetFilesInDirectoryException& e) {
        m_error = e.what();
    }
    FillModelDataCheckingForRoot(dir);
    endResetModel();
}

void FileListViewModel::RefreshDirectory(const QString& dir) { ChangeDirectory(dir); }

std::optional<std::string> FileListViewModel::ClaimError() {
    auto temp = m_error;
    m_error = {};
    return temp;
}

template <typename FileContainer, typename DirContainer>
static auto CombineAllEntriesIntoNameVec(const FileContainer& files, const DirContainer& dirs) {
    std::vector<QString> names;
    std::transform(begin(files), end(files), std::back_inserter(names),
                   [&](const auto& file) { return FromRawPath(FileSystem::GetFilename(file)); });
    std::transform(begin(dirs), end(dirs), std::back_inserter(names),
                   [&](const auto& dir) { return FromRawPath(FileSystem::GetDirectoryname(dir)); });
    return names;
}

template <typename FileContainer, typename DirContainer>
static auto CombineAllEntriesIntoFullPathsVec(const FileContainer& files, const DirContainer& dirs) {
    std::vector<QString> fullPaths;
    std::transform(begin(files), end(files), std::back_inserter(fullPaths),
                   [&](const auto& file) { return FromRawPath(file.path()); });
    std::transform(begin(dirs), end(dirs), std::back_inserter(fullPaths),
                   [&](const auto& dir) { return FromRawPath(dir.path()); });
    return fullPaths;
}

static bool DirIsRoot(const QString& dir) {
    const auto rawDir = ToRawPath(dir);
    return rawDir == FileSystem::_getRootPath(rawDir);
}

void FileListViewModel::FillModelDataCheckingForRoot(const QString& dir) {
    FillModelData();

    if (!DirIsRoot(dir)) {
        m_displayName.insert(m_displayName.begin(), "..");
        m_displaySize.insert(m_displaySize.begin(), "");
        m_displayDateModified.insert(m_displayDateModified.begin(), "");
        m_fullPaths.insert(m_fullPaths.begin(), "..");
        m_icons.insert(m_icons.begin(), m_styleProvider.standardIcon(QStyle::SP_DirIcon));
    }
}

static auto FilePathsFromEntries(const std::vector<FileListViewModel::FileEntry>& entries) {
    std::vector<FileSystem::FilePath> paths;
    std::transform(entries.begin(), entries.end(), std::back_inserter(paths),
                   [](const auto& entry) { return entry.filePath; });
    return paths;
}

static QString FormatSize(std::uintmax_t bytes) {
    return QString::fromStdString(
        FileSystem::Algorithm::Format(FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(bytes), 0));
}

static QString FormatSize(std::time_t timestamp) {
    return QString::fromStdString(FileSystem::Algorithm::FormatAsFileTimestamp(timestamp));
}

void FileListViewModel::FillModelData() {
    m_displayName = CombineAllEntriesIntoNameVec(FilePathsFromEntries(m_fileEntries), m_directoryEntries);

    std::transform(m_fileEntries.begin(), m_fileEntries.end(), std::back_inserter(m_displaySize),
                   [](const auto& entry) { return FormatSize(entry.size); });

    const std::vector<QString> dummyStrings(m_directoryEntries.size(), "");
    m_displaySize.insert(m_displaySize.end(), dummyStrings.begin(), dummyStrings.end());

    std::transform(m_fileEntries.begin(), m_fileEntries.end(), std::back_inserter(m_displayDateModified),
                   [](const auto& entry) { return FormatSize(entry.lastModificationDate); });

    m_displayDateModified.insert(m_displayDateModified.end(), dummyStrings.begin(), dummyStrings.end());

    m_fullPaths = CombineAllEntriesIntoFullPathsVec(FilePathsFromEntries(m_fileEntries), m_directoryEntries);
    FillIcons();
}

void FileListViewModel::FillIcons() {
    const auto dirIcon = m_styleProvider.standardIcon(QStyle::SP_DirIcon);
    const auto fileIcon = m_styleProvider.standardIcon(QStyle::SP_FileIcon);

    m_icons.clear();

    int index = -1;

    std::transform(m_fullPaths.begin(), m_fullPaths.end(), std::back_inserter(m_icons), [&](const auto& fullPath) {
        const auto fullPathStdString = ToRawPath(fullPath);
        ++index;
        if (FileSystem::IsDirectory(fullPathStdString)) {
            return dirIcon;
        } else if (FileSystem::IsRegularFile(fullPathStdString)) {
            StartIconRetrievalThread(fullPathStdString, index);
            return fileIcon;
        }

        return QIcon();
    });
}

void FileListViewModel::StartIconRetrievalThread(const std::wstring& fullPathStdString, int index) {
    if (AssociatedIconProvider::ComponentIsAvailable() /*Don't start making threads that don't do anything*/) {
        auto* runnable = new IconRetrievalRunnable(fullPathStdString, index);
        connect(runnable, &IconRetrievalRunnable::resultReady, this, &FileListViewModel::RefreshIcon);
        QThreadPool::globalInstance()->start(runnable);
    }
}

int FileListViewModel::rowCount(const QModelIndex&) const { return static_cast<int>(m_displayName.size()); }

QVariant FileListViewModel::headerData(int section, Qt::Orientation, int role) const {
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

void FileListViewModel::RefreshIcon(QIcon icon, const QString& fullPath, int index) {
    if (m_fullPaths.empty())
        return;

    if (m_fullPaths.front() == "..")
        ++index;

    /*Don't update unless it's still the right filepath for the given index
    Also don't update when the index is out of range*/
    if (index < m_icons.size() && index < m_fullPaths.size() && m_fullPaths[index] == fullPath)
        setData(createIndex(index, 0), icon, Qt::DecorationRole);
}
