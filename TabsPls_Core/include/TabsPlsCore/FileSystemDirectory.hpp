#pragma once

#include <string>
#include <optional>

#include "FileSystem.hpp"

namespace FileSystem
{
	class FilePath;

	/*! \brief Cannot be constructed unless a valid path was given. So only a check at contruction is needed.*/
	class Directory
	{
	public:
		static Directory FromCurrentWorkingDirectory();
		static std::optional<Directory> FromPath(const FileSystem::RawPath& path);
		
		static Directory FromFilePathParent(const FilePath& path);

		Directory(const Directory&) = default;
		Directory(Directory&&) = default;

		Directory& operator=(Directory other);

		Directory Parent() const;

		auto& path() const { return m_path; }

		friend void swap(Directory& first, Directory& second);
	protected:
		Directory(RawPath path) : m_path(std::move(path)) {}

	private:
		FileSystem::RawPath m_path;
	};
}


