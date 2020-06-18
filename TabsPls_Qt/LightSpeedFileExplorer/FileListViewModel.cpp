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

template<typename FileContainer, typename DirContainer>
static auto CombineAllEntriesIntoNameVec(const FileContainer& files, const DirContainer& dirs)
{
    std::vector<QString> names;
    std::transform(begin(files), end(files), std::back_inserter(names), [&](const auto& file) {return QString::fromStdString(FileSystem::GetFilename(file)); });
    std::transform(begin(dirs), end(dirs), std::back_inserter(names), [&](const auto& dir) {return QString::fromStdString(FileSystem::GetDirectoryname(dir)); });
    return names;
}

FileListViewModel::FileListViewModel(const QString& initialDirectory)
{
    std::tie(m_fileEntries, m_directoryEntries) = RetrieveDirectoryContents(initialDirectory);
    m_display = CombineAllEntriesIntoNameVec(m_fileEntries, m_directoryEntries);
}

QVariant FileListViewModel::data(const QModelIndex& index, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        return m_display[index.row()];
    default:
        break;
    }

    return {};
}

void FileListViewModel::ChangeDirectory(const QString& dir)
{
    beginResetModel();
    std::tie(m_fileEntries, m_directoryEntries) = RetrieveDirectoryContents(dir);
    m_display = CombineAllEntriesIntoNameVec(m_fileEntries, m_directoryEntries);
    endResetModel();
}

int FileListViewModel::rowCount(const QModelIndex&) const
{
    return m_display.size();
}

QVariant FileListViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section >= mockupTableHeaders.size())
        return {};

    switch (role) {
    case Qt::DisplayRole:
        return mockupTableHeaders[section];
    default:
        break;
    }

    return {};
}
