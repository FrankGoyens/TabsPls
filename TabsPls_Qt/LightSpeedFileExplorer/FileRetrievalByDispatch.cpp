#include "FileRetrievalByDispatch.hpp"

#include <QStyle>

#include <TabsPlsCore/FileSystemAlgorithm.hpp>

#include <FileSystemDefsConversion.hpp>

using FileSystem::StringConversion::FromRawPath;
using FileSystem::StringConversion::ToRawPath;

namespace FileRetrievalByDispatch {

std::pair<std::vector<FileSystem::FilePath>, std::vector<FileSystem::Directory>>
RetrieveFilesAndDirectories(const FileSystem::Directory& dir) {
    std::vector<FileSystem::FilePath> files;
    std::vector<FileSystem::Directory> dirs;

    for (const auto& entry : FileSystem::GetFilesInDirectory(dir)) {
        if (const auto file = FileSystem::FilePath::FromPath(entry))
            files.push_back(*file);
        else if (const auto dirEntry = FileSystem::Directory::FromPath(entry))
            dirs.push_back(*dirEntry);
    }

    return std::make_pair(files, dirs);
}

std::vector<FileEntryModel::FileEntry> RetrieveFiles(const FileSystem::Directory& dir,
                                                     const DirectoryReadDispatcher& dispatcher) {
    const auto [files, dirs] = RetrieveFilesAndDirectories(dir);

    for (const auto& childDir : dirs) {
        dispatcher.DirectoryReadDispatch(childDir);
    }
    return FileEntryModel::FilesAsModelEntries(files);
}

FileSystem::RawPath SubtractBasePath(const FileSystem::RawPath& basePath, const FileSystem::RawPath& filePath) {
    if (basePath.size() > filePath.size())
        return {};

    return FileSystem::Algorithm::StripLeadingPathSeparators({filePath.begin() + basePath.size(), filePath.end()});
}

FileEntryModel::ModelEntry AsModelEntry(const FileEntryModel::FileEntry& fileEntry, const FileSystem::RawPath& basePath,
                                        const QStyle& styleProvider) {
    return AsModelEntry(fileEntry, basePath, styleProvider.standardIcon(QStyle::SP_FileIcon));
}

FileEntryModel::ModelEntry AsModelEntry(const FileEntryModel::FileEntry& fileEntry, const FileSystem::RawPath& basePath,
                                        const QIcon& fileIcon) {
    const auto fullPath = fileEntry.filePath.path();
    return {FromRawPath(SubtractBasePath(basePath, fullPath)), FileEntryModel::FormatSize(fileEntry.size),
            FileEntryModel::FormatDateLastModified(fileEntry.lastModificationDate), FromRawPath(fullPath), fileIcon};
}

std::vector<FileEntryModel::ModelEntry> AsModelEntries(const std::vector<FileEntryModel::FileEntry>& fileEntries,
                                                       const FileSystem::RawPath& basePath, QStyle& styleProvider) {
    return AsModelEntries(fileEntries, basePath, styleProvider.standardIcon(QStyle::SP_FileIcon));
}

std::vector<FileEntryModel::ModelEntry> AsModelEntries(const std::vector<FileEntryModel::FileEntry>& fileEntries,
                                                       const FileSystem::RawPath& basePath, const QIcon& fileIcon) {
    std::vector<FileEntryModel::ModelEntry> modelEntries;
    std::transform(fileEntries.begin(), fileEntries.end(), std::back_inserter(modelEntries),
                   [&](const auto& fileEntry) { return AsModelEntry(fileEntry, basePath, fileIcon); });
    return modelEntries;
}

} // namespace FileRetrievalByDispatch