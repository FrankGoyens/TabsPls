# This cmake macro fills two variables TabsPls_Core_Headers and TabsPls_Core_Sources with core header and sources respectively
macro(ProvisionCoreSources prefix)

    set(TabsPls_Core_Headers 
        ${prefix}/include/TabsPlsCore/FileSystemDirectory.hpp
        ${prefix}/include/TabsPlsCore/DirectoryHistoryStore.hpp
        ${prefix}/include/TabsPlsCore/FileSystemFilePath.hpp
        ${prefix}/include/TabsPlsCore/FileSystem.hpp
        ${prefix}/include/TabsPlsCore/FileSystemOp.hpp
        ${prefix}/include/TabsPlsCore/RobustDirectoryHistoryStore.hpp
        ${prefix}/include/TabsPlsCore/CurrentDirectoryFileOp.hpp
	${prefix}/include/TabsPlsCore/FileSystemAlgorithm.hpp
	${prefix}/include/TabsPlsCore/Send2Trash.hpp
	${prefix}/include/TabsPlsCore/ProgressReport.hpp
	${prefix}/include/TabsPlsCore/TargetDirectoryConstraints.hpp
	${prefix}/include/TabsPlsCore/DirectoryInputAutoComplete.hpp
	${prefix}/include/TabsPlsCore/TabModel.hpp

    )

    set(TabsPls_Core_Sources
        ${prefix}/source/FileSystemDirectory.cpp
        ${prefix}/source/FileSystemFilePath.cpp
        ${prefix}/source/DirectoryHistoryStore.cpp
        ${prefix}/source/RobustDirectoryHistoryStore.cpp
        ${prefix}/source/CurrentDirectoryFileOp.cpp
	${prefix}/source/FileSystemAlgorithm.cpp
	${prefix}/source/TargetDirectoryConstraints.cpp
	${prefix}/source/DirectoryInputAutoComplete.cpp
	${prefix}/source/TabModel.cpp
    )

endmacro()
