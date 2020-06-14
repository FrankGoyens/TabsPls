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
    ChangeDirectory(initialDirectory);
}

template<typename FileContainer, typename DirContainer>
static auto CombineAllEntriesIntoNameVec(const FileContainer& files, const DirContainer& dirs)
{
    std::vector<QString> names;
    std::transform(begin(files), end(files), std::back_inserter(names), [&](const auto& file) {return QString::fromStdString(FileSystem::GetFilename(file)); });
    std::transform(begin(dirs), end(dirs), std::back_inserter(names), [&](const auto& dir) {return QString::fromStdString(FileSystem::GetDirectoryname(dir)); });
    return names;
}

QVariant FileListViewModel::data(const QModelIndex& index, int role) const
{
    const auto names = CombineAllEntriesIntoNameVec(m_fileEntries, m_directoryEntries);

    switch (role) {
    case Qt::DisplayRole:
        return names[index.row()];
    default:
        break;
    }

    return {};
}

void FileListViewModel::ChangeDirectory(const QString& dir)
{
    std::tie(m_fileEntries, m_directoryEntries) = RetrieveDirectoryContents(dir);
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
