#pragma once

#include <QIcon>
#include <QString>

#include <TabsPlsCore/FileSystemFilePath.hpp>

namespace FileEntryModel {

struct ModelEntry {
    QString displayName;
    QString displaySize;
    QString displayDateModified;
    QString fullPath;
    QIcon icon;
};

struct FileEntry {
    FileSystem::FilePath filePath;
    std::time_t lastModificationDate;
    std::uintmax_t size;
};

std::vector<FileEntry> FilesAsModelEntries(const std::vector<FileSystem::FilePath>& files);

ModelEntry AsModelEntry(const FileEntry& fileEntry);

std::vector<FileSystem::FilePath> FilePathsFromEntries(const std::vector<FileEntryModel::FileEntry>& entries);

QString FormatSize(std::uintmax_t bytes);

QString FormatDateLastModified(std::time_t timestamp);

} // namespace FileEntryModel
