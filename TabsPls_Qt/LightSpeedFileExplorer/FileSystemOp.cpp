#include <TabsPlsCore/FileSystemOp.hpp>

#include <filesystem>

#include <TabsPlsCore/FileSystem.hpp>
#include <TabsPlsCore/FileSystemAlgorithm.hpp>
#include <TabsPlsCore/FileSystemDirectory.hpp>

namespace FileSystem {
namespace Op {
void CopyRecursive(const RawPath& source, const RawPath& dest) {
    try {
        std::filesystem::copy(source, dest,
                              std::filesystem::copy_options::recursive);
    } catch (const std::filesystem::filesystem_error& e) {
        throw CopyException{e.what()};
    }
}

void Rename(const RawPath& source, const RawPath& dest) {
    if (std::filesystem::exists(dest))
        throw RenameException("This name already exists");

    try {
        std::filesystem::rename(source, dest);
    } catch (const std::filesystem::filesystem_error& e) {
        throw RenameException(e.what());
    }
}

void RemoveAll(const RawPath& dest) {
    try {
        std::filesystem::remove_all(dest);
    } catch (const std::filesystem::filesystem_error& e) {
    }
}

void CreateDirectory(const Directory& parent, const Name& newDirName) {
    try {
        const auto newDirPath =
            FileSystem::Algorithm::CombineDirectoryAndName(parent, newDirName);
        std::filesystem::create_directory(newDirPath);
    } catch (const std::filesystem::filesystem_error& e) {
        throw CreateDirectoryException(e.what());
    }
}
} // namespace Op
} // namespace FileSystem
