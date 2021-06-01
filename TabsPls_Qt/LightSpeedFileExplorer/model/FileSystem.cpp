#include <TabsPlsCore/FileSystem.hpp>

#include <filesystem>

#include <TabsPlsCore/FileSystemDirectory.hpp>
#include <TabsPlsCore/FileSystemFilePath.hpp>

namespace FileSystem {
Name Separator() {
    // A bit hacky, but on Windows the separator is a wchar_t and on Linux it's
    // not So let the std::filesystem api insert the separator and make it into
    // an std::wstring
    static const auto dummyPath = (std::filesystem::path("C") / "").wstring();
    static const auto sep = dummyPath.substr(1, dummyPath.length() - 1);
    return sep;
}

bool IsDirectory(const RawPath& dir) {
    std::error_code error;
    const bool result = std::filesystem::is_directory(dir, error);
    if (error)
        return false;
    return result;
}

bool IsRegularFile(const RawPath& path) {
    std::error_code error;
    const bool result = std::filesystem::is_regular_file(path, error);
    if (error)
        return false;
    return result;
}

RawPath RemoveFilename(const FilePath& filePath) {
    return std::filesystem::path(filePath.path()).remove_filename().wstring();
}

Name GetFilename(const FilePath& filePath) { return std::filesystem::path(filePath.path()).filename().wstring(); }

Name GetDirectoryname(const Directory& dir) {
    const std::filesystem::path dirPath(dir.path());
    const std::filesystem::path parentDirPath(dir.Parent().path());

    return dirPath.lexically_relative(parentDirPath).wstring();
}

Name _getRootPath(const RawPath& path) { return std::filesystem::path(path).root_path().wstring(); }

Name _getRootName(const RawPath& path) { return std::filesystem::path(path).root_name().wstring(); }

RawPath GetWorkingDirectory() { return std::filesystem::current_path().wstring(); }

RawPathVector GetFilesInCurrentDirectory() { return _getFilesInDirectory(GetWorkingDirectory()); }

RawPathVector GetFilesInDirectory(const Directory& dir) { return _getFilesInDirectory(dir.path()); }

RawPathVector _getFilesInDirectory(const RawPath& dir) {
    RawPathVector files;
    for (auto& it : std::filesystem::directory_iterator(dir))
        files.push_back(it.path().wstring());

    return files;
}

RawPath GetParent(const Directory& dir) {
    std::filesystem::path dirPath = dir.path();

    return dirPath.parent_path().wstring();
}

std::uintmax_t GetFileSize(const FilePath& file) { return std::filesystem::file_size(file.path()); }

// https://stackoverflow.com/questions/61030383/how-to-convert-stdfilesystemfile-time-type-to-time-t
template <typename TimePoint> static std::time_t to_time_t(TimePoint tp) {
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(tp - TimePoint::clock::now() +
                                                                                  std::chrono::system_clock::now());
    return std::chrono::system_clock::to_time_t(sctp);
}

std::time_t GetLastWriteTime(const FilePath& file) {
    const auto fileTime = std::filesystem::last_write_time(file.path());
    return to_time_t(fileTime);
}
} // namespace FileSystem