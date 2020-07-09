#include <TabsPlsCore/FileSystemOp.hpp>

#include <filesystem>

namespace FileSystem
{
	namespace Op
	{
		void CopyRecursive(const RawPath& source, const RawPath& dest)
		{
			try {
				std::filesystem::copy(std::filesystem::path(source), std::filesystem::path(dest), std::filesystem::copy_options::recursive);
			}
			catch (const std::filesystem::filesystem_error& e) {
				throw CopyException{ e.what() };
			}
		}
	}
}