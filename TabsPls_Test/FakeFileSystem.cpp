#include <TabsPlsCore/FileSystem.hpp>

#include <unordered_map>
#include <unordered_set>
#include <sstream>

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

        std::vector<std::shared_ptr<FileSystemNode>> rootNodes;
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
}