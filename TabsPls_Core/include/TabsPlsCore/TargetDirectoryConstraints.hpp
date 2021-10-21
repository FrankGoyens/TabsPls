#pragma once

#include <FileSystemDefs.hpp>

namespace TargetDirectoryConstraints {

/**! \brief For some reason, paths like "C:" are accepted, but iterating through them yiels the current directory's
 * contents. Very strange.
 * */
bool IsIncompleteWindowsRootPath(FileSystem::RawPath path);
} // namespace TargetDirectoryConstraints