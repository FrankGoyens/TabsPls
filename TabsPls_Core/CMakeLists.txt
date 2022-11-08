project(TabsPls_Core)

set(TabsPls_Core_Headers 
	${CMAKE_CURRENT_SOURCE_DIR}/include/TabsPlsCore/FileSystemDirectory.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/TabsPlsCore/DirectoryHistoryStore.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/TabsPlsCore/FileSystemFilePath.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/TabsPlsCore/FileSystem.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/TabsPlsCore/FileSystemOp.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/TabsPlsCore/RobustDirectoryHistoryStore.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/TabsPlsCore/CurrentDirectoryFileOp.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/TabsPlsCore/FileSystemAlgorithm.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/TabsPlsCore/Send2Trash.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/TabsPlsCore/ProgressReport.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/TabsPlsCore/TargetDirectoryConstraints.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/TabsPlsCore/DirectoryInputAutoComplete.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/TabsPlsCore/TabModel.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/TabsPlsCore/EmbeddedPython.hpp

	${CMAKE_CURRENT_SOURCE_DIR}/include/TabsPlsCore/exception/TabsPlsException.hpp
)

set(TabsPls_Core_Sources
	${CMAKE_CURRENT_SOURCE_DIR}/source/FileSystemDirectory.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/FileSystemFilePath.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/DirectoryHistoryStore.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/RobustDirectoryHistoryStore.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/CurrentDirectoryFileOp.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/FileSystemAlgorithm.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/TargetDirectoryConstraints.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/DirectoryInputAutoComplete.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/TabModel.cpp
)

add_library(TabsPls_Core INTERFACE)
target_sources(TabsPls_Core INTERFACE ${TabsPls_Core_Headers} ${TabsPls_Core_Sources})
target_include_directories(TabsPls_Core INTERFACE include)
