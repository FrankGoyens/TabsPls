#include <TabsPlsCore/CurrentDirectoryFileOp.hpp>

#include <TabsPlsCore/FileSystem.hpp>
#include <TabsPlsCore/FileSystemOp.hpp>
#include <TabsPlsCore/FileSystemAlgorithm.hpp>

void CurrentDirectoryFileOp::CopyRecursive(const FileSystem::RawPath& source, const FileSystem::Name& destName)
{
	const auto destParent = FileSystem::Algorithm::StripTrailingPathSeparators(GetCurrentDir().path());
	FileSystem::Op::CopyRecursive(source, destParent + FileSystem::Separator() + FileSystem::Algorithm::StripLeadingPathSeparators(destName));
}

void CurrentDirectoryFileOp::Move(const FileSystem::RawPath& source, const FileSystem::Name& destName)
{
	const auto destParent = FileSystem::Algorithm::StripTrailingPathSeparators(GetCurrentDir().path());
	FileSystem::Op::Rename(source, destParent + FileSystem::Separator() + FileSystem::Algorithm::StripLeadingPathSeparators(destName));
}
