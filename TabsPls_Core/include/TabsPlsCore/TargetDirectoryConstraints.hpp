#pragma once

#include <FileSystemDefs.hpp>

namespace TargetDirectoryConstraints {

bool DirIsRoot(const FileSystem::RawPath& dir);

/**! \brief For some reason, paths like "C:" are accepted, but iterating through them yields the current directory's
 * contents. Very strange.
 * */
bool IsIncompleteWindowsRootPath(FileSystem::RawPath path);
} // namespace TargetDirectoryConstraints