#include "EscapePodLauncher.hpp"

#include <QDebug>
#include <QProcess>
#include <QUrl>

#include <TabsPlsCore/FileSystemDirectory.hpp>

#include "FileSystemDefsConversion.hpp"

namespace EscapePodLauncher {
void LaunchUrlInWorkingDirectory(const QUrl& url, const FileSystem::Directory& workingDir) {
    const auto workingDirQString = FileSystem::StringConversion::FromRawPath(workingDir.path());
    const int status =
        QProcess::execute(QString("EscapePod"), QStringList::fromVector({url.toEncoded(), workingDirQString}));

    if (status != 0)
        qDebug() << "Something went wrong launching url " << url.toString() << " in directory " << workingDirQString;
}
} // namespace EscapePodLauncher
