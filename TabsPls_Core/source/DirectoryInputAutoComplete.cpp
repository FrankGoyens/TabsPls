#include <TabsPlsCore/DirectoryInputAutoComplete.hpp>

#include <algorithm>
#include <iterator>
#include <set>

#include <TabsPlsCore/FileSystemDirectory.hpp>

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

namespace {
struct ForwardAutoCompletionVariant {};
struct BackwardAutoCompletionVariant {};
} // namespace

static FileSystem::RawPath
AutoCompleteUsingAllSortedOptions(const std::set<FileSystem::RawPath>& sortedDirectories,
                                  const std::set<FileSystem::RawPath>::const_iterator& entryToComplete,
                                  const ForwardAutoCompletionVariant&) {
    const auto autoCompleter =
        std::next(entryToComplete) == sortedDirectories.end() ? sortedDirectories.begin() : std::next(entryToComplete);
    return *autoCompleter;
}

static FileSystem::RawPath
AutoCompleteUsingAllSortedOptions(const std::set<FileSystem::RawPath>& sortedDirectories,
                                  const std::set<FileSystem::RawPath>::const_iterator& entryToComplete,
                                  const BackwardAutoCompletionVariant&) {
    if (entryToComplete == sortedDirectories.begin())
        return *std::prev(sortedDirectories.end());
    return *std::prev(entryToComplete);
}

template <typename CompletionVariant>
static FileSystem::RawPath AutoCompleteInBaseDir(const FileSystem::Directory& baseDir,
                                                 const FileSystem::RawPath& incompletePath,
                                                 const CompletionVariant& completionVariant) {
    const auto directories = GetDirectories(baseDir);
    std::set<FileSystem::RawPath> sortedDirectories;
    std::transform(directories.begin(), directories.end(), std::inserter(sortedDirectories, sortedDirectories.end()),
                   [](const auto& dir) { return dir.path(); });
    auto insertion = sortedDirectories.insert(incompletePath);
    return AutoCompleteUsingAllSortedOptions(sortedDirectories, insertion.first, completionVariant);
}

template <typename CompletionVariant>
std::optional<FileSystem::RawPath> AutoCompleteUsingRawIncompletePath(const FileSystem::RawPath& incompletePath,
                                                                      const CompletionVariant& completionVariant) {
    const auto lastSeparator =
        std::find(incompletePath.rbegin(), incompletePath.rend(), FileSystem::Separator().front());
    const FileSystem::RawPath incompleteNewPart = Reverse({incompletePath.rbegin(), lastSeparator});
    const auto basePathForIncompletePart = incompletePath.substr(0, incompletePath.size() - incompleteNewPart.size());
    if (const auto baseDir = FileSystem::Directory::FromPath(basePathForIncompletePart)) {
        return AutoCompleteInBaseDir(*baseDir, incompletePath, completionVariant);
    }
    return {};
}

namespace DirectoryInputAutoComplete {
std::optional<FileSystem::RawPath> Do(const FileSystem::RawPath& incompletePath) {
    return AutoCompleteUsingRawIncompletePath(incompletePath, ForwardAutoCompletionVariant{});
}

std::optional<FileSystem::RawPath> DoReverse(const FileSystem::RawPath& incompletePath) {
    return AutoCompleteUsingRawIncompletePath(incompletePath, BackwardAutoCompletionVariant{});
}

} // namespace DirectoryInputAutoComplete