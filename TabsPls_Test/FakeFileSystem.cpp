#include <TabsPlsCore/FileSystem.hpp>
#include <TabsPlsCore/FileSystemOp.hpp>

#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <algorithm>

#include <filesystem>
#include "FakeFileSystem.hpp"

#include <TabsPlsCore/FileSystemDirectory.hpp>
#include <TabsPlsCore/FileSystemFilePath.hpp>

namespace
{
    struct FileSystemNode
    {
        FileSystemNode(FileSystem::Name nodeName_) :
            nodeName(std::move(nodeName_))
        {}

        FileSystemNode(const FileSystemNode& other):
            nodeName(other.nodeName),
            files(other.files),
            parentNode(other.parentNode)
        {
            std::transform(other.childNodes.begin(), other.childNodes.end(), std::back_inserter(childNodes), 
                [](const auto& childNodePtr) {return std::make_shared<FileSystemNode>(*childNodePtr); });
        }

        FileSystemNode(FileSystemNode&& other) = default;

        FileSystemNode& operator=(FileSystemNode other)
        {
            swap(*this, other);
            return *this;
        }

        void swap(FileSystemNode& first, FileSystemNode& second)
        {
            using std::swap;
            swap(first.nodeName, second.nodeName);
            swap(first.files, second.files);
            swap(first.parentNode, second.parentNode);
            swap(first.childNodes, second.childNodes);
        }

        FileSystem::Name nodeName;
        std::vector<FileSystem::Name> files;

        std::weak_ptr<FileSystemNode> parentNode;
        std::vector<std::shared_ptr<FileSystemNode>> childNodes;
    };

    template<typename ContainerT>
    typename ContainerT::const_iterator FindChildNode(const ContainerT& childSharedPtrs, const FileSystem::Name& childName)
    {
        return std::find_if(childSharedPtrs.begin(), childSharedPtrs.end(), [&](const auto& childNode) {return childNode->nodeName == childName; });
    }

    void CreateOrUpdateNodeStructure(const std::shared_ptr<FileSystemNode>& parentNode, const std::vector<FileSystem::Name>& absoluteComponents)
    {
        std::shared_ptr<FileSystemNode> currentParent = parentNode;
        for (const auto& component : absoluteComponents)
        {
            const auto it = FindChildNode(currentParent->childNodes, component);
            
            if (it != currentParent->childNodes.end())
            {
                currentParent = *it;
                continue;
            }

            currentParent->childNodes.push_back(std::make_shared<FileSystemNode>(component));
            currentParent->childNodes.back()->parentNode = currentParent;
            currentParent = currentParent->childNodes.back();
        }
    }

    class FakeFileSystemImpl
    {
    public:
        static FakeFileSystemImpl& Instance() 
        {
            static FakeFileSystemImpl impl;
            return impl;
        }

        void Clear()
        {
            rootNodes.clear();
        }
        
        void AddDirectory(const std::vector<FileSystem::Name>& absoluteComponents)
        {
            if (absoluteComponents.empty())
                return;

            UpdateRootNodeStructure(absoluteComponents);
        }

        void AddFile(const std::vector<FileSystem::Name>& parentAbsoluteComponents, const FileSystem::Name& fileName)
        {
            if (parentAbsoluteComponents.empty())
                return;

            UpdateRootNodeStructure(parentAbsoluteComponents);
            if (auto createdTail = Lookup(parentAbsoluteComponents))
                createdTail->files.push_back(fileName);
            else
                throw std::logic_error("A newly created structure somehow failed because the new tail does not exist");
        }

        void DeleteDirectory(const std::vector<FileSystem::Name>& absoluteComponents)
        {
            if (absoluteComponents.empty())
                return;

            if (absoluteComponents.size() == 1) {
                rootNodes.erase(
                    std::remove_if(rootNodes.begin(), rootNodes.end(), [&](const auto& rootNode) {return absoluteComponents.front() == rootNode->nodeName; }),
                    rootNodes.end()
                );
            }
            else if (const auto node = Lookup(absoluteComponents)) {
                DeleteNonRootNode(*node);
            }
        }

        void DeleteNode(const FileSystemNode& node)
        {
            const auto it = std::find_if(rootNodes.begin(), rootNodes.end(), [&](const auto& rootNode) {return &node == rootNode.get(); });
            
            if (it != rootNodes.end())
            {
                rootNodes.erase(it);
                return;
            }

            DeleteNonRootNode(node);
        }

        void DeleteFile(const std::vector<FileSystem::Name>& parentAbsoluteComponents, const FileSystem::Name& fileName)
        {
            if (const auto node = Lookup(parentAbsoluteComponents))
            {
                node->files.erase(
                    std::remove(node->files.begin(), node->files.end(), fileName),
                    node->files.end()
                );
            }
        }

        std::shared_ptr<FileSystemNode> Lookup(const std::vector<FileSystem::Name>& parentAbsoluteComponents)
        {
            if (parentAbsoluteComponents.empty())
                return nullptr;

            const auto rootIt = std::find_if(rootNodes.begin(), rootNodes.end(), [&](const auto& rootNode) {return rootNode->nodeName == parentAbsoluteComponents.front(); });
            if (rootIt==rootNodes.end())
                return nullptr;

            std::shared_ptr<FileSystemNode> currentParent = *rootIt;
			for (auto it = parentAbsoluteComponents.begin() + 1; it != parentAbsoluteComponents.end(); ++it)
            {
                const auto childIt = FindChildNode(currentParent->childNodes, *it);
                if (childIt == currentParent->childNodes.end())
                    return nullptr;
                currentParent = *childIt;
            }
            return currentParent;
        }

    private:
        FakeFileSystemImpl() = default;

        std::vector<std::shared_ptr<FileSystemNode>> rootNodes;

        void UpdateRootNodeStructure(const std::vector<FileSystem::Name>& absoluteComponents)
        {
            const auto rootIt = std::find_if(rootNodes.begin(), rootNodes.end(), [&](const auto& rootNode) {return rootNode->nodeName == absoluteComponents.front(); });

            if(rootIt==rootNodes.end())
            {
                const auto newRootNode = std::make_shared<FileSystemNode>(absoluteComponents.front());
                rootNodes.push_back(newRootNode);
                CreateOrUpdateNodeStructure(newRootNode, { absoluteComponents.begin() + 1, absoluteComponents.end() });
            }
            else
                CreateOrUpdateNodeStructure(*rootIt, { absoluteComponents.begin() + 1, absoluteComponents.end() });
        }

        void DeleteNonRootNode(const FileSystemNode& node)
        {
            if (const auto parentNode = node.parentNode.lock())
            {
                parentNode->childNodes.erase(
                    std::remove_if(parentNode->childNodes.begin(), parentNode->childNodes.end(), [&](const auto& childNode) {return childNode.get() == &node; }),
                    parentNode->childNodes.end());
            }
        }
    };
}

static auto SplitUsingSeparator(FileSystem::RawPath path)
{
    std::vector<FileSystem::Name> components;

    auto sep = FakeFileSystem::GetSeparator();
    FileSystem::RawPath::size_type pos;

    while ((pos = path.find(sep)) != FileSystem::RawPath::npos)
    {
        components.push_back(path.substr(0, pos));
        path.erase(0, pos + sep.length());
    }

    components.push_back(path);
    return components;
}

static FileSystem::RawPath MergeUsingSeparator(const std::vector<FileSystem::Name>& components)
{
    if (components.empty())
        return "";

    const auto sep = FakeFileSystem::GetSeparator();
    std::ostringstream outStream;
    for (auto it = components.begin(); it != components.end() - 1; ++it)
        outStream << *it << sep;
    outStream << components.back();

    return outStream.str();
}

static auto SplitParentDirAndFilename(const FileSystem::FilePath& file)
{
    const auto parentDir = FileSystem::Directory::FromFilePathParent(file).path();
    const auto filename = FileSystem::GetFilename(file);

    return std::make_pair(parentDir, filename);
}

namespace FakeFileSystem
{
    void Cleanup()
    {
        FakeFileSystemImpl::Instance().Clear();
    }
    
    void AddDirectory(const std::initializer_list<FileSystem::Name>& absoluteComponents)
    {
        FakeFileSystemImpl::Instance().AddDirectory(absoluteComponents);
    }
    
    void AddFile(const std::initializer_list<FileSystem::Name>& parentAbsoluteComponents, const FileSystem::Name& fileName)
    {
        FakeFileSystemImpl::Instance().AddFile(parentAbsoluteComponents, fileName);
    }

    FileSystem::RawPath MergeUsingSeparator(const std::vector<FileSystem::Name>& components)
    {
        return ::MergeUsingSeparator(components);
    }
    
    FileSystem::Name GetSeparator()
    {
        return "/";
    }
}

namespace FileSystem
{

    bool IsDirectory(const RawPath& dir)
    {
        return FakeFileSystemImpl::Instance().Lookup(SplitUsingSeparator(dir)) != nullptr;
    }

	bool IsRegularFile(const RawPath& path)
	{
        const auto components = SplitUsingSeparator(path);
        if (auto parentDir = FakeFileSystemImpl::Instance().Lookup(std::vector<FileSystem::Name>(components.begin(), components.end() - 1)))
        {
            const auto it = std::find(parentDir->files.begin(), parentDir->files.end(), components.back());
            return it != parentDir->files.end();
        }
        return false;
	}

    RawPath RemoveFilename(const FilePath& filePath)
    {
        const auto components = SplitUsingSeparator(filePath.path());

        return MergeUsingSeparator({ components.begin(), components.end() - 1 });
    }

    Name GetFilename(const FilePath& filePath)
    {
        const auto components = SplitUsingSeparator(filePath.path());

        return components.back();
    }

    Name GetDirectoryname(const Directory& dir)
    {
        const auto components = SplitUsingSeparator(dir.path());

        return components.back();
    }

    Name _getRootPath(const RawPath& path)
    {
        const auto components = SplitUsingSeparator(path);

        return components.front() + FakeFileSystem::GetSeparator();
    }

    Name _getRootName(const RawPath& path)
    {
        const auto components = SplitUsingSeparator(path);

        return components.front();
    }

    RawPath GetWorkingDirectory()
    {
        return std::filesystem::current_path().string(); //not really worthwile to fake at this point
    }

    RawPathVector GetFilesInCurrentDirectory()
    {
        return _getFilesInDirectory(GetWorkingDirectory()); //not really worthwile to fake at this point
    }

    RawPathVector GetFilesInDirectory(const Directory& dir)
    {
        return _getFilesInDirectory(dir.path());
    }

    RawPathVector _getFilesInDirectory(const RawPath& dir)
    {
        if (const auto node = FakeFileSystemImpl::Instance().Lookup(SplitUsingSeparator(dir)))
        {
            auto entries = node->files;
            std::transform(node->childNodes.begin(), node->childNodes.end(), std::back_inserter(entries), [](const auto& childNode) {return childNode->nodeName; });
            return entries;
        }

        return {};
    }
    
    RawPath GetParent(const Directory& dir)
    {
        auto components = SplitUsingSeparator(dir.path());

        if (components.size() > 1)
            components.pop_back();

        return MergeUsingSeparator(components);
    }

    namespace Op
    {
        static void RenameFile(const FilePath& sourceFile, const RawPath& dest) {
            const auto destSplit = SplitUsingSeparator(dest);

            FakeFileSystemImpl::Instance().AddFile({ destSplit.begin(), destSplit.end() - 1 }, destSplit.back());
            auto [sourceParent, sourceFilename] = SplitParentDirAndFilename(sourceFile);
            FakeFileSystemImpl::Instance().DeleteFile(SplitUsingSeparator(sourceParent), sourceFilename);
        }

        static void RenameDir(const FileSystemNode& sourceNode, const RawPath& dest) {
            const auto destSplit = SplitUsingSeparator(dest);
            
            FakeFileSystemImpl::Instance().DeleteNode(sourceNode);
            FakeFileSystemImpl::Instance().AddDirectory(destSplit);
            if (const auto destNode = FakeFileSystemImpl::Instance().Lookup(destSplit)) {
                const auto destName = destNode->nodeName;
                *destNode = sourceNode;
                destNode->nodeName = destName;
            }
            else
                throw std::logic_error("Rename Error: A newly created structure somehow failed because the new tail does not exist");
        }

        static void RenameWhereDestDoesNotExist(const RawPath& source, const RawPath& dest)
        {
            if (const auto sourceNode = FakeFileSystemImpl::Instance().Lookup(SplitUsingSeparator(source)))
                RenameDir(*sourceNode, dest);
            else if (const auto sourceFile = FileSystem::FilePath::FromPath(source)) 
                RenameFile(*sourceFile, dest);
            else
                throw RenameException("The source has an unknown type.");
        }

        static void RenameExistingSource(const RawPath& source, const RawPath& dest)
        {
            if (IsRegularFile(dest) || IsDirectory(dest)) {
                throw RenameException("The destination already exists.");
            }
            else {
                RenameWhereDestDoesNotExist(source, dest);
            }
        }

        void Rename(const RawPath& source, const RawPath& dest)
        {
            if (source == dest)
                return;

            if (IsRegularFile(source) || IsDirectory(source)) {
                RenameExistingSource(source, dest);
            }
            else {
                throw RenameException{ "The source does not exist." };
            }
        }

        static void CopyFile(const FilePath& sourceFile, const RawPath& dest) {
            const auto destSplit = SplitUsingSeparator(dest);

            FakeFileSystemImpl::Instance().AddFile({ destSplit.begin(), destSplit.end() - 1 }, destSplit.back());
        }

        static void CopyDir(const FileSystemNode& sourceNode, const RawPath& dest) {
            const auto destSplit = SplitUsingSeparator(dest);

            FakeFileSystemImpl::Instance().AddDirectory(destSplit);
            if (const auto destNode = FakeFileSystemImpl::Instance().Lookup(destSplit)) {
                const auto destParentNode = destNode->parentNode;
                const auto destName = destNode->nodeName;
                *destNode = sourceNode;
                destNode->parentNode = destParentNode;
                destNode->nodeName = destName;
            }
            else
                throw std::logic_error("Copy Error: A newly created structure somehow failed because the new tail does not exist");
        }

        static void CopyWhereDestDoesNotExist(const RawPath& source, const RawPath& dest)
        {
            if (const auto sourceNode = FakeFileSystemImpl::Instance().Lookup(SplitUsingSeparator(source)))
                CopyDir(*sourceNode, dest);
            else if (const auto sourceFile = FileSystem::FilePath::FromPath(source))
                CopyFile(*sourceFile, dest);
            else
                throw CopyException("The source has an unknown type.");
        }

        static void CopyExistingSourceRecursive(const RawPath& source, const RawPath& dest)
        {
            if (IsRegularFile(dest) || IsDirectory(dest)) {
                throw CopyException("The destination already exists.");
            }
            else {
                CopyWhereDestDoesNotExist(source, dest);
            }
        }

        void CopyRecursive(const RawPath& source, const RawPath& dest)
        {
            if (IsRegularFile(source) || IsDirectory(source)) {
                CopyExistingSourceRecursive(source, dest);
            }
            else {
                throw CopyException{ "The source does not exist." };
            }
        }

        static void RemoveDir(const RawPath& path)
        {
            FakeFileSystemImpl::Instance().DeleteDirectory(SplitUsingSeparator(path));
        }

        static void RemoveFile(const FilePath& file)
        {
            auto [parent, name] = SplitParentDirAndFilename(file);
            FakeFileSystemImpl::Instance().DeleteFile(SplitUsingSeparator(parent), name);
        }

        void RemoveAll(const RawPath& path)
        {
            if (IsDirectory(path))
                RemoveDir(path);
            else if (const auto file = FilePath::FromPath(path))
                RemoveFile(*file);
        }
    }
}
