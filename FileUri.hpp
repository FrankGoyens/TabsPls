#pragma once

#include <string>
#include <optional>

namespace FileSystem
{
	class FilePath;
	class Directory;
}

/*! \brief Cannot be constructed unless a valid URI was given. So only a check at contruction is needed.*/
class FileUri
{
public:
	static std::optional<FileUri> FromURIString(const std::string&);
	
	static FileUri FromFilePath(const FileSystem::FilePath&);
	static FileUri FromDirectory(const FileSystem::Directory&);

	virtual ~FileUri() = default;
	FileUri(const FileUri&) = default;
	FileUri& operator=(FileUri);

	const std::string& string() const { return m_uriString; }

	friend void swap(FileUri& first, FileUri& second);
private:
	FileUri(std::string uriString) :
		m_uriString(std::move(uriString))
	{}

	std::string m_uriString;
};
