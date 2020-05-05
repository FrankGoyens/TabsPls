cmake_minimum_required(VERSION 3.11)

project(TabsPls)

set(TabsPlsCorePrefix ${CMAKE_SOURCE_DIR}/../TabsPls_Core)

set(TabsPls_Headers SmartGtk.hpp
    ${TabsPlsCorePrefix}/include/TabsPlsCore/FileSystemDirectory.hpp
    ${TabsPlsCorePrefix}/include/TabsPlsCore/DirectoryHistoryStore.hpp
    ${TabsPlsCorePrefix}/include/TabsPlsCore/FileSystemFilePath.hpp
    ${TabsPlsCorePrefix}/include/TabsPlsCore/FileSystem.hpp
    
    TabsPlsCore_defs/pch.h
    TabsPlsCore_defs/FileSystemDefs.hpp

    FileUri.hpp
    CurrentDirectoryProvider.hpp
    DirectoryHistory.hpp

	GtkGui/FileListView.hpp
	GtkGui/DragAndDrop.hpp
	GtkGui/DirectoryNavigationField.hpp
	GtkGui/DirectoryHistoryButtons.hpp
    
    Gui/DirectoryChangedAction.hpp
)

set(TabsPls_Sources ${TabsPlsCorePrefix}/source/FileSystemDirectory.cpp
    ${TabsPlsCorePrefix}/source/FileSystemFilePath.cpp
    ${TabsPlsCorePrefix}/source/DirectoryHistoryStore.cpp
    FileUri.cpp

    GtkGui/FileListView.cpp
    GtkGui/DragAndDrop.cpp
	GtkGui/DirectoryNavigationField.cpp
	GtkGui/DirectoryHistoryButtons.cpp

    model/FileSystem.cpp
)

add_library(TabsPls ${TabsPls_Headers} ${TabsPls_Sources})
target_include_directories(TabsPls PUBLIC .)
target_include_directories(TabsPls PUBLIC ${TabsPlsCorePrefix}/include)
target_include_directories(TabsPls PUBLIC TabsPlsCore_defs)

find_package(PkgConfig)
if(PkgConfig_FOUND)
    PKG_CHECK_MODULES(GTK3 gtk+-3.0)
    target_include_directories(TabsPls PUBLIC ${GTK3_INCLUDE_DIRS})
    target_link_libraries(TabsPls PUBLIC ${GTK3_LIBRARIES})
endif()

find_package(tl-expected CONFIG)
if(tl-expected_FOUND)
	target_link_libraries(TabsPls PUBLIC tl::expected)
endif()

find_package(skyr-url CONFIG)
if(skyr-url_FOUND)
	target_link_libraries(TabsPls PUBLIC skyr::skyr-url)
endif()

target_compile_features(TabsPls PUBLIC cxx_std_17)

add_subdirectory(test)

add_subdirectory(TabsPlsMain)
