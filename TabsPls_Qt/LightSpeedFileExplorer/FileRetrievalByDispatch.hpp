#pragma once

#include <TabsPlsCore/FileSystemDirectory.hpp>

#include "FileEntryModel.hpp"

class QStyle;
class QIcon;

namespace FileRetrievalByDispatch {
struct DirectoryReadDispatcher {
    virtual ~DirectoryReadDispatcher() = default;
    virtual void DirectoryReadDispatch(const FileSystem::Directory&) const = 0;
};

std::pair<std::vector<FileSystem::FilePath>, std::vector<FileSystem::Directory>>
RetrieveFilesAndDirectories(const FileSystem::Directory& dir);

std::vector<FileEntryModel::FileEntry> RetrieveFiles(const FileSystem::Directory& dir,
                                                     const DirectoryReadDispatcher& dispatcher);

FileSystem::RawPath SubtractBasePath(const FileSystem::RawPath& basePath, const FileSystem::RawPath& filePath);

FileEntryModel::ModelEntry AsModelEntry(const FileEntryModel::FileEntry& fileEntry, const FileSystem::RawPath& basePath,
                                        const QStyle& styleProvider);

FileEntryModel::ModelEntry AsModelEntry(const FileEntryModel::FileEntry& fileEntry, const FileSystem::RawPath& basePath,
                                        const QIcon& fileIcon);

std::vector<FileEntryModel::ModelEntry> AsModelEntries(const std::vector<FileEntryModel::FileEntry>& fileEntries,
                                                       const FileSystem::RawPath& basePath, QStyle& styleProvider);

std::vector<FileEntryModel::ModelEntry> AsModelEntries(const std::vector<FileEntryModel::FileEntry>& fileEntries,
                                                       const FileSystem::RawPath& basePath, const QIcon&);

} // namespace FileRetrievalByDispatch