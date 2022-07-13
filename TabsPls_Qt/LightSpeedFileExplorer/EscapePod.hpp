#pragma once

namespace FileSystem {
class Directory;
}

class QUrl;

namespace EscapePod {
void LaunchUrlInWorkingDirectory(const QUrl&, const FileSystem::Directory& workingDir);
}
