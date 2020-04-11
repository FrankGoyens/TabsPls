#pragma once

#include <string>
#include <optional>
#include <model/FileSystem.hpp>

namespace FileSystem
{
	/*! \brief Cannot be constructed unless a valid path was given. So only a check at contruction is needed.*/
	class Directory
	{
	public:
		static Directory FromCurrentWorkingDirectory();
		static std::optional<Directory> FromPath(const FileSystem::RawPath& path);

		Directory(const Directory&) = default;
		Directory(Directory&&) = default;

		Directory& operator=(Directory other);

		Directory Parent() const;

		auto& path() const { return m_path; }

		friend void swap(Directory& first, Directory& second);
	private:
		Directory(RawPath path) : m_path(std::move(path)) {}

		FileSystem::RawPath m_path;
	};
}


