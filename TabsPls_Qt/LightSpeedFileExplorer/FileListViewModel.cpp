#include "FileListViewModel.hpp"

#include <TabsPlsCore/FileSystem.hpp>
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
    FillModelData();
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

void FileListViewModel::ChangeDirectory(const QString& dir)
{
    beginResetModel();
    std::tie(m_fileEntries, m_directoryEntries) = RetrieveDirectoryContents(dir);
    FillModelData();
    endResetModel();
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

void FileListViewModel::FillModelData()
{
    m_display = CombineAllEntriesIntoNameVec(m_fileEntries, m_directoryEntries);
    m_fullPaths = CombineAllEntriesIntoFullPathsVec(m_fileEntries, m_directoryEntries);
    
    m_display.insert(m_display.begin(), "..");
    m_fullPaths.insert(m_fullPaths.begin(), "..");
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
