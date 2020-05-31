# This cmake macro fills two variables TabsPls_Core_Headers and TabsPls_Core_Sources with core header and sources respectively
macro(ProvisionCoreSources prefix)

    set(TabsPls_Core_Headers 
        ${prefix}/include/TabsPlsCore/FileSystemDirectory.hpp
        ${prefix}/include/TabsPlsCore/DirectoryHistoryStore.hpp
        ${prefix}/include/TabsPlsCore/FileSystemFilePath.hpp
        ${prefix}/include/TabsPlsCore/FileSystem.hpp
        ${prefix}/include/TabsPlsCore/RobustDirectoryHistoryStore.hpp
    )

    set(TabsPls_Core_Sources
        ${prefix}/source/FileSystemDirectory.cpp
        ${prefix}/source/FileSystemFilePath.cpp
        ${prefix}/source/DirectoryHistoryStore.cpp
        ${prefix}/source/RobustDirectoryHistoryStore.cpp
    )

endmacro()