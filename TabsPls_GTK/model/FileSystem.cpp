#include <TabsPlsCore/FileSystem.hpp>

#include <filesystem>

#include <TabsPlsCore/FileSystemDirectory.hpp>
#include <TabsPlsCore/FileSystemFilePath.hpp>

namespace FileSystem
{

    bool IsDirectory(const RawPath& dir)
    {
        std::error_code error;
        const bool result = std::filesystem::is_directory(dir, error);
        if (error)
            return false;
        return result;
    }

	bool IsRegularFile(const RawPath& path)
	{
        std::error_code error;
		const bool result = std::filesystem::is_regular_file(path, error);
        if (error)
            return false;
        return result;
	}

    RawPath RemoveFilename(const FilePath& filePath)
    {
        return std::filesystem::path(filePath.path()).remove_filename().string();
    }

    Name GetFilename(const FilePath& filePath)
    {
        return std::filesystem::path(filePath.path()).filename().string();
    }

    Name GetDirectoryname(const Directory& dir)
    {
        const std::filesystem::path dirPath(dir.path());
        const std::filesystem::path parentDirPath(dir.Parent().path());

        return dirPath.lexically_relative(parentDirPath).string();
    }

    Name _getRootPath(const RawPath& path)
    {
        return std::filesystem::path(path).root_path().string();
    }

    Name _getRootName(const RawPath& path)
    {
        return std::filesystem::path(path).root_name().string();
    }

    RawPath GetWorkingDirectory()
    {
        return std::filesystem::current_path().string();
    }

    RawPathVector GetFilesInCurrentDirectory()
    {
        return _getFilesInDirectory(GetWorkingDirectory());
    }

    RawPathVector GetFilesInDirectory(const Directory& dir)
    {
        return _getFilesInDirectory(dir.path());
    }

    RawPathVector _getFilesInDirectory(const RawPath& dir)
    {
        RawPathVector files;
        for(auto& it: std::filesystem::directory_iterator(dir))
            files.push_back(it.path().string());

        return files;
    }
    
    RawPath GetParent(const Directory& dir)
    {
        std::filesystem::path dirPath = dir.path();

        return dirPath.parent_path().string();
    }
}