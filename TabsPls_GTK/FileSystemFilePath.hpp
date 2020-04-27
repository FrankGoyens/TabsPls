#pragma once

#include <string>
#include <optional>
#include <model/FileSystem.hpp>

namespace FileSystem
{
	/*! \brief Cannot be constructed unless a valid path was given. So only a check at contruction is needed.*/
	class FilePath
	{
	public:
		static std::optional<FilePath> FromPath(const FileSystem::RawPath& path);

		FilePath(const FilePath&) = default;
		FilePath(FilePath&&) = default;

		FilePath& operator=(FilePath other);

		auto& path() const { return m_path; }

		friend void swap(FilePath& first, FilePath& second);
	private:
		FilePath(RawPath path) : m_path(std::move(path)) {}

		FileSystem::RawPath m_path;
	};
}


