#include "FileListViewModel.hpp"

#include <TabsPlsCore/FileSystem.hpp>
#include <TabsPlsCore/FileSystemAlgorithm.hpp>
#include <TabsPlsCore/FileSystemOp.hpp>
#include <TabsPlsCore/FileSystemDirectory.hpp>

namespace
{
    const std::vector<QString> mockupTableHeaders =
    {
        {
            {QObject::tr("Name")}
        }
    };
}

static auto RetrieveDirectoryContents(const QString& initialDirectory)
{
    std::vector<FileSystem::FilePath> files;
    std::vector<FileSystem::Directory> dirs;

    if (auto dir = FileSystem::Directory::FromPath(initialDirectory.toStdString()))
    {
        for (const auto& entry : FileSystem::GetFilesInDirectory(*dir))
        {
            if (auto file = FileSystem::FilePath::FromPath(entry))
            {
                files.emplace_back(*file);
                continue;
            }

            if(auto dir = FileSystem::Directory::FromPath(entry))
                dirs.emplace_back(*dir);
        }
    }
    
    return std::make_pair(files, dirs);
}

FileListViewModel::FileListViewModel(const QString& initialDirectory)
{
    std::tie(m_fileEntries, m_directoryEntries) = RetrieveDirectoryContents(initialDirectory);
    FillModelDataCheckingForRoot(initialDirectory);
}

QVariant FileListViewModel::data(const QModelIndex& index, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        return m_display[index.row()];
    case Qt::UserRole:
        return m_fullPaths[index.row()];
    default:
        break;
    }

    return {};
}

bool FileListViewModel::setData(const QModelIndex& index, const QVariant& value, int)
{
    const auto fullPathAtIndex = data(index, Qt::UserRole).toString().toStdString();
   
    std::optional<std::pair<FileSystem::RawPath, FileSystem::RawPath>> renameCall = {};
    FileSystem::RawPath parentPath;

    if (const auto dir = FileSystem::Directory::FromPath(fullPathAtIndex)) {
        parentPath = FileSystem::Algorithm::StripTrailingPathSeparators(dir->Parent().path());
        if (FileSystem::Algorithm::StripTrailingPathSeparators(dir->path()) == parentPath)
            return false;
        
        renameCall = std::make_pair(dir->path(), parentPath + FileSystem::Separator() + FileSystem::Algorithm::StripLeadingPathSeparators(value.toString().toStdString()));
    }
    else if (const auto file = FileSystem::FilePath::FromPath(fullPathAtIndex)) {
        parentPath = FileSystem::Algorithm::StripTrailingPathSeparators(FileSystem::Directory::FromFilePathParent(*file).path());
        renameCall = std::make_pair(file->path(), parentPath + FileSystem::Separator() + FileSystem::Algorithm::StripLeadingPathSeparators(value.toString().toStdString()));
    }

    if (!renameCall)
        return false;

    try {
        FileSystem::Op::Rename(renameCall->first, renameCall->second);
        RefreshDirectory(parentPath.c_str());
    }
    catch (const FileSystem::Op::RenameException& e) {
        m_error = e.message;
        return false;
    }

    return true;
}

Qt::ItemFlags FileListViewModel::flags(const QModelIndex& index) const
{
    if (index.column() == 1 && index.row() == 0)
        return QAbstractTableModel::flags(index);
    return QAbstractTableModel::flags(index) | Qt::ItemFlag::ItemIsEditable;
}

void FileListViewModel::ChangeDirectory(const QString& dir)
{
    beginResetModel();
    std::tie(m_fileEntries, m_directoryEntries) = RetrieveDirectoryContents(dir);
    FillModelDataCheckingForRoot(dir);
    endResetModel();
}

void FileListViewModel::RefreshDirectory(const QString& dir)
{
    ChangeDirectory(dir);
}

std::optional<std::string> FileListViewModel::ClaimError()
{
    auto temp = m_error;
    m_error = {};
    return temp;
}

template<typename FileContainer, typename DirContainer>
static auto CombineAllEntriesIntoNameVec(const FileContainer& files, const DirContainer& dirs)
{
    std::vector<QString> names;
    std::transform(begin(files), end(files), std::back_inserter(names), [&](const auto& file) {return QString::fromStdString(FileSystem::GetFilename(file)); });
    std::transform(begin(dirs), end(dirs), std::back_inserter(names), [&](const auto& dir) {return QString::fromStdString(FileSystem::GetDirectoryname(dir)); });
    return names;
}

template<typename FileContainer, typename DirContainer>
static auto CombineAllEntriesIntoFullPathsVec(const FileContainer& files, const DirContainer& dirs)
{
    std::vector<QString> fullPaths;
    std::transform(begin(files), end(files), std::back_inserter(fullPaths), [&](const auto& file) {return QString::fromStdString(file.path()); });
    std::transform(begin(dirs), end(dirs), std::back_inserter(fullPaths), [&](const auto& dir) {return QString::fromStdString(dir.path()); });
    return fullPaths;
}

static bool DirIsRoot(const QString& dir)
{
    return dir.toStdString() == FileSystem::_getRootPath(dir.toStdString());
}

void FileListViewModel::FillModelDataCheckingForRoot(const QString& dir)
{
    FillModelData();

    if (!DirIsRoot(dir))
    {
        m_display.insert(m_display.begin(), "..");
        m_fullPaths.insert(m_fullPaths.begin(), "..");
    }
}

void FileListViewModel::FillModelData()
{
    m_display = CombineAllEntriesIntoNameVec(m_fileEntries, m_directoryEntries);
    m_fullPaths = CombineAllEntriesIntoFullPathsVec(m_fileEntries, m_directoryEntries);   
}

int FileListViewModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(m_display.size());
}

QVariant FileListViewModel::headerData(int section, Qt::Orientation, int role) const
{
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
