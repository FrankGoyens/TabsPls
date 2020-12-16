#include <TabsPlsCore/FileSystemDirectory.hpp>

#include <TabsPlsCore/FileSystemFilePath.hpp>

namespace FileSystem
{
    Directory Directory::FromCurrentWorkingDirectory()
    {
        return Directory(FileSystem::GetWorkingDirectory());
    }

    std::optional<Directory> Directory::FromPath(const FileSystem::RawPath& path)
    {
        if (!IsDirectory(path))
            return {};

        return Directory(path);
    }

	Directory Directory::FromFilePathParent(const FilePath& filePath)
	{
        const auto parentDir = RemoveFilename(filePath);
		return Directory(parentDir);
	}

    Directory& Directory::operator=(Directory other)
    {
        swap(*this, other);
        return *this;
    }

    Directory Directory::Parent() const
    {
        return Directory(FileSystem::GetParent(*this));
    }
	
    void swap(Directory& first, Directory& second)
	{
        using std::swap;
        swap(first.m_path, second.m_path);
	}
}