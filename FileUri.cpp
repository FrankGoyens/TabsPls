#include "FileUri.hpp"

#include <skyr/url.hpp>

std::optional<FileUri> FileUri::FromString(const std::string& uri)
{
	const auto fileUri = skyr::url(uri);
	
	if (fileUri.protocol() != "file:")
		return {};

	return FileUri(uri);
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
