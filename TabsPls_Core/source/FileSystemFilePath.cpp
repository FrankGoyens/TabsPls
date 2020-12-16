#include <TabsPlsCore/FileSystemFilePath.hpp>

namespace FileSystem
{
	std::optional<FilePath> FilePath::FromPath(const FileSystem::RawPath& path)
	{
		if (FileSystem::IsRegularFile(path))
			return FilePath(path);

		return {};
	}

	FilePath& FilePath::operator=(FilePath other)
	{
		swap(*this, other);
		return *this;
	}
	
	void swap(FilePath& first, FilePath& second)
	{
		using std::swap;
		swap(first.m_path, second.m_path);
	}
}
