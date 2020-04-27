#include "FileUri.hpp"

#include <sstream>

#include <skyr/url.hpp>

#include <FileSystemFilePath.hpp>
#include <FileSystemDirectory.hpp>

std::optional<FileUri> FileUri::FromURIString(const std::string& uri)
{
	const auto fileUri = skyr::url(uri);
	
	if (fileUri.protocol() != "file:")
		return {};

	if (fileUri.hostname() != "")
		return {}; //Currently only local files are supported

	return FileUri(uri);
}

template<typename ComponentIterableT>
static auto ComposePathFromComponents(const ComponentIterableT& iterable)
{
	std::ostringstream path;
	path << "file:///";

	for (auto it = iterable.rbegin(); it != iterable.rend() - 1; ++it)
		path << *it << "/";

	path << iterable.front();
	return path.str();
}

static auto ComponentsStartingFromParentDirectory(const FileSystem::Directory& dir)
{
	std::vector<std::string> pathComponents;

	const auto rootPath = FileSystem::_getRootPath(dir.path());
	auto currentParent = dir.Parent();

	while (currentParent.path() != rootPath)
	{
		pathComponents.push_back(FileSystem::GetDirectoryname(currentParent));
		currentParent = currentParent.Parent();
	}

	pathComponents.push_back(FileSystem::_getRootName(dir.path()));
	return pathComponents;
}

FileUri FileUri::FromFilePath(const FileSystem::FilePath& filePath)
{
	const auto pathComponents = ComponentsStartingFromParentDirectory(FileSystem::Directory::FromFilePathParent(filePath));

	return FileUri(ComposePathFromComponents(pathComponents) + "/" + FileSystem::GetFilename(filePath));
}

FileUri FileUri::FromDirectory(const FileSystem::Directory& dir)
{
	const auto pathComponents = ComponentsStartingFromParentDirectory(dir);

	return FileUri(ComposePathFromComponents(pathComponents) + "/" + FileSystem::GetDirectoryname(dir));
}

FileUri& FileUri::operator=(FileUri other)
{
	swap(*this, other);
	return *this;
}

void swap(FileUri& first, FileUri& second)
{
	using std::swap;
	swap(first.m_uriString, second.m_uriString);
}
