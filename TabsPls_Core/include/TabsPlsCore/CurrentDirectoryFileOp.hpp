#pragma once

#include <TabsPlsCore/FileSystemDirectory.hpp>

class CurrentDirectoryFileOp {
  public:
    virtual ~CurrentDirectoryFileOp() = default;

    // Throws the same exceptions as FileSystem::Op::CopyRecursive
    void CopyRecursive(const FileSystem::RawPath& source, const FileSystem::Name& destName);

    // Throws the same exceptions as FileSystem::Op::Move
    void Move(const FileSystem::RawPath& source, const FileSystem::Name& destName);

    virtual FileSystem::Directory GetCurrentDir() const = 0;
};