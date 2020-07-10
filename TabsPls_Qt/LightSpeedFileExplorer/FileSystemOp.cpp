#include <TabsPlsCore/FileSystemOp.hpp>

#include <filesystem>

#include <TabsPlsCore/FileSystem.hpp>

namespace FileSystem
{
	namespace Op
	{
		void CopyRecursive(const RawPath& source, const RawPath& dest)
		{
			try {
				std::filesystem::copy(source, dest, std::filesystem::copy_options::recursive);
			}
			catch (const std::filesystem::filesystem_error& e) {
				throw CopyException{ e.what() };
			}
		}

		void Rename(const RawPath& source, const RawPath& dest)
		{
			if(std::filesystem::exists(dest))
				throw RenameException("This name already exists");

			try {
				std::filesystem::rename(source, dest);
			}
			catch (const std::filesystem::filesystem_error& e) {
				throw RenameException(e.what());
			}
		}

		void RemoveAll(const RawPath& dest)
		{
			try {
				std::filesystem::remove_all(dest);
			}
			catch (const std::filesystem::filesystem_error& e) {}
		}
	}
}