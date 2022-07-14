#pragma once

namespace FileSystem {
class Directory;
}

class QUrl;

namespace EscapePodLauncher {
void LaunchUrlInWorkingDirectory(const QUrl&, const FileSystem::Directory& workingDir);
}
