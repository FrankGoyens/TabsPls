#include "FileEntryModel.hpp"

#include <TabsPlsCore/FileSystemAlgorithm.hpp>

#include "FileSystemDefsConversion.hpp"

namespace FileEntryModel {

static bool ModelEntryDisplayNameSortingPredicate(const ModelEntry& first, const ModelEntry& second) {
    return first.displayName < second.displayName;
}

bool ModelEntryDepthSortingPredicate(const ModelEntry& first, const ModelEntry& second) {
    static auto separatorAsQString = FileSystem::StringConversion::FromRawPath(FileSystem::Separator());
    const auto firstDepth = first.displayName.count(separatorAsQString);
    const auto secondDepth = second.displayName.count(separatorAsQString);
    if (firstDepth == secondDepth)
        return ModelEntryDisplayNameSortingPredicate(first, second);
    return firstDepth < secondDepth;
}

std::vector<FileEntryModel::FileEntry> FilesAsModelEntries(const std::vector<FileSystem::FilePath>& files) {
    std::vector<FileEntryModel::FileEntry> filesWithSizes;
    std::transform(files.begin(), files.end(), std::back_inserter(filesWithSizes), [](const auto& file) {
        return FileEntryModel::FileEntry{file, FileSystem::GetLastWriteTime(file), FileSystem::GetFileSize(file)};
    });
    return filesWithSizes;
}

std::vector<FileSystem::FilePath> FilePathsFromEntries(const std::vector<FileEntryModel::FileEntry>& entries) {
    std::vector<FileSystem::FilePath> paths;
    std::transform(entries.begin(), entries.end(), std::back_inserter(paths),
                   [](const auto& entry) { return entry.filePath; });
    return paths;
}

QString FormatSize(std::uintmax_t bytes) {
    return QString::fromStdString(
        FileSystem::Algorithm::Format(FileSystem::Algorithm::ScaleSizeToLargestPossibleUnit(bytes), 0));
}

QString FormatDateLastModified(std::time_t timestamp) {
    return QString::fromStdString(FileSystem::Algorithm::FormatAsFileTimestamp(timestamp));
}
} // namespace FileEntryModel
