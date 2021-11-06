#include "DirectoryInputField.hpp"

#include <set>
#include <stdexcept>

#include <QKeyEvent>
#include <QLineEdit>

#include "FileSystemDefsConversion.hpp"

#include <TabsPlsCore/FileSystem.hpp>
#include <TabsPlsCore/FileSystemDirectory.hpp>
#include <TabsPlsCore/TargetDirectoryConstraints.hpp>

using FileSystem::StringConversion::FromRawPath;
using FileSystem::StringConversion::ToRawPath;

DirectoryInputField::DirectoryInputField(QString initialDirectory) : m_currentDirectory(std::move(initialDirectory)) {
    if (!FileSystem::IsDirectory(ToRawPath(m_currentDirectory)))
        throw std::invalid_argument("The given directory is not valid");

    setText(m_currentDirectory);

    connect(this, &QLineEdit::editingFinished, [this]() {
        if (text() == m_currentDirectory)
            return;

        const auto newDirectory = ToRawPath(text());
        if (!FileSystem::IsDirectory(newDirectory)) {
            setText(m_currentDirectory);
            return;
        }

        if (TargetDirectoryConstraints::IsIncompleteWindowsRootPath(newDirectory)) {
            setText(m_currentDirectory);
            return;
        }

        m_currentDirectory = text();
        emit directoryChanged(m_currentDirectory);
    });
}

static auto Reverse(const FileSystem::RawPath& rawPath) {
    return FileSystem::RawPath(rawPath.rbegin(), rawPath.rend());
}

static auto GetDirectories(const FileSystem::Directory& dir) {
    std::vector<FileSystem::Directory> directories;
    for (const auto& item : FileSystem::GetFilesInDirectory(dir)) {
        if (const auto possibleDir = FileSystem::Directory::FromPath(item)) {
            directories.emplace_back(*possibleDir);
        }
    }
    return directories;
}

void DirectoryInputField::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Tab) {
        const auto incompletePath = ToRawPath(text());
        const auto lastSeparator =
            std::find(incompletePath.rbegin(), incompletePath.rend(), FileSystem::Separator().front());
        const FileSystem::RawPath incompleteNewPart = Reverse({incompletePath.rbegin(), lastSeparator});
        const auto basePathForIncompletePart =
            incompletePath.substr(0, incompletePath.size() - incompleteNewPart.size());
        if (const auto baseDir = FileSystem::Directory::FromPath(basePathForIncompletePart)) {
            const auto directories = GetDirectories(*baseDir);
            std::set<FileSystem::RawPath> sortedDirectories;
            std::transform(directories.begin(), directories.end(),
                           std::inserter(sortedDirectories, sortedDirectories.end()),
                           [](const auto& dir) { return dir.path(); });
            auto insertion = sortedDirectories.insert(incompletePath);
            const auto autoCompleter = std::next(insertion.first) == sortedDirectories.end()
                                           ? sortedDirectories.begin()
                                           : std::next(insertion.first);
            setText(FromRawPath(*autoCompleter));
        }
        return;
    }
    QLineEdit::keyPressEvent(event);
}
