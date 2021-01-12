#include "FileListViewModel.hpp"

#include <QStyle>

#include <TabsPlsCore/FileSystem.hpp>
#include <TabsPlsCore/FileSystemAlgorithm.hpp>
#include <TabsPlsCore/FileSystemDirectory.hpp>
#include <TabsPlsCore/FileSystemOp.hpp>

#include "FileSystemDefsConversion.hpp"

using FileSystem::StringConversion::FromRawPath;
using FileSystem::StringConversion::ToRawPath;

namespace {
const std::vector<QString> mockupTableHeaders = {{{QObject::tr("Name")}}};

struct GetFilesInDirectoryException : std::exception {
    GetFilesInDirectoryException(const char* message_) : message(message_) {}

    const char* what() const noexcept override { return message.c_str(); }

    std::string message;
};
} // namespace

static std::pair<std::vector<FileSystem::FilePath>, std::vector<FileSystem::Directory>>
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

        return std::make_pair(files, dirs);
    } catch (const std::exception& e) {
        throw GetFilesInDirectoryException(e.what());
    }

    return {{}, {}};
}

static std::pair<std::vector<FileSystem::FilePath>, std::vector<FileSystem::Directory>>
RetrieveDirectoryContents(const QString& initialDirectory) {
    if (auto dir = FileSystem::Directory::FromPath(ToRawPath(initialDirectory)))
        return GetFilesInDirectoryWithFSCatch(*dir);

    return {{}, {}};
}

FileListViewModel::FileListViewModel(QStyle& styleProvider, const QString& initialDirectory)
    : m_styleProvider(styleProvider) {
    std::tie(m_fileEntries, m_directoryEntries) = RetrieveDirectoryContents(initialDirectory);
    FillModelDataCheckingForRoot(initialDirectory);
}

QVariant FileListViewModel::data(const QModelIndex& index, int role) const {
    switch (role) {
    case Qt::EditRole:
    // fallthrough
    case Qt::DisplayRole:
        return m_display[index.row()];
    case Qt::UserRole:
        return m_fullPaths[index.row()];
    case Qt::DecorationRole:
        return m_icons[index.row()];
    default:
        break;
    }

    return {};
}

bool FileListViewModel::setData(const QModelIndex& index, const QVariant& value, int) {
    const auto fullPathAtIndex = ToRawPath(data(index, Qt::UserRole).toString());

    std::optional<std::pair<FileSystem::RawPath, FileSystem::RawPath>> renameCall = {};
    FileSystem::RawPath parentPath;

    if (const auto dir = FileSystem::Directory::FromPath(fullPathAtIndex)) {
        parentPath = FileSystem::Algorithm::StripTrailingPathSeparators(dir->Parent().path());
        if (FileSystem::Algorithm::StripTrailingPathSeparators(dir->path()) == parentPath)
            return false;

        renameCall = std::make_pair(dir->path(),
                                    parentPath + FileSystem::Separator() +
                                        FileSystem::Algorithm::StripLeadingPathSeparators(ToRawPath(value.toString())));
    } else if (const auto file = FileSystem::FilePath::FromPath(fullPathAtIndex)) {
        parentPath =
            FileSystem::Algorithm::StripTrailingPathSeparators(FileSystem::Directory::FromFilePathParent(*file).path());
        renameCall = std::make_pair(file->path(),
                                    parentPath + FileSystem::Separator() +
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
}

Qt::ItemFlags FileListViewModel::flags(const QModelIndex& index) const {
    if (index.column() == 0 && index.row() == 0)
        return QAbstractTableModel::flags(index);
    return QAbstractTableModel::flags(index) | Qt::ItemFlag::ItemIsEditable;
}

void FileListViewModel::ChangeDirectory(const QString& dir) {
    beginResetModel();
    try {
        std::tie(m_fileEntries, m_directoryEntries) = RetrieveDirectoryContents(dir);
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
        m_display.insert(m_display.begin(), "..");
        m_fullPaths.insert(m_fullPaths.begin(), "..");
        m_icons.insert(m_icons.begin(), m_styleProvider.standardIcon(QStyle::SP_DirIcon));
    }
}

void FileListViewModel::FillModelData() {
    m_display = CombineAllEntriesIntoNameVec(m_fileEntries, m_directoryEntries);
    m_fullPaths = CombineAllEntriesIntoFullPathsVec(m_fileEntries, m_directoryEntries);

    const auto dirIcon = m_styleProvider.standardIcon(QStyle::SP_DirIcon);
    const auto fileIcon = m_styleProvider.standardIcon(QStyle::SP_FileIcon);

    m_icons.clear();

    std::transform(m_fullPaths.begin(), m_fullPaths.end(), std::back_inserter(m_icons), [&](const auto& fullPath) {
        const auto fullPathStdString = ToRawPath(fullPath);
        if (FileSystem::IsDirectory(fullPathStdString))
            return dirIcon;
        else if (FileSystem::IsRegularFile(fullPathStdString))
            return fileIcon;

        return QIcon();
    });
}

int FileListViewModel::rowCount(const QModelIndex&) const { return static_cast<int>(m_display.size()); }

QVariant FileListViewModel::headerData(int section, Qt::Orientation, int role) const {
    if (static_cast<unsigned>(section) >= mockupTableHeaders.size())
        return {};

    switch (role) {
    case Qt::DisplayRole:
        return mockupTableHeaders[section];
    default:
        break;
    }

    return {};
}
