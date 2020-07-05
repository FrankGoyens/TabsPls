#pragma once

#include <vector>
#include <string>

#include <FileSystemDefs.hpp>

namespace FileSystem
{
	namespace Op
	{
		struct FileSystemOpException : std::exception {
			FileSystemOpException(std::string what_) : message(std::move(what_)) {}
			const char* what() const noexcept override { return message.c_str(); }

			std::string message;
		};
		
		struct CopyException : FileSystemOpException {
			CopyException(std::string what) : FileSystemOpException(std::move(what)) {}
		};

		void CopyRecursive(const RawPath& source, const RawPath& dest);

		struct RenameException : FileSystemOpException {
			RenameException(std::string what) : FileSystemOpException(std::move(what)) {}
		};

		void Rename(const RawPath& source, const RawPath& dest);

		void RemoveAll(const RawPath&);
	}
}