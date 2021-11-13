#pragma once

#include <optional>

#include <FileSystemDefs.hpp>

namespace DirectoryInputAutoComplete {
std::optional<FileSystem::RawPath> Do(const FileSystem::RawPath& incompletePath);
std::optional<FileSystem::RawPath> DoReverse(const FileSystem::RawPath& incompletePath);
} // namespace DirectoryInputAutoComplete