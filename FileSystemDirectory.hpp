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
		static std::optional<Directory> FromPath(const FileSystem::RawPath& path);

		auto& path() const { return m_path; }
	private:
		Directory(RawPath path) : m_path(std::move(path)) {}

		FileSystem::RawPath m_path;
	};
}


