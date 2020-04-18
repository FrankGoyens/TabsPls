#pragma once

#include <string>
#include <optional>

/*! \brief Cannot be constructed unless a valid URI was given. So only a check at contruction is needed.*/
class FileUri
{
public:
	static std::optional<FileUri> FromString(const std::string& uri);

	virtual ~FileUri() = default;
	FileUri(const FileUri&) = default;
	FileUri& operator=(FileUri);

	friend void swap(FileUri& first, FileUri& second);
private:
	FileUri(std::string);

	std::string m_uriString;
};
