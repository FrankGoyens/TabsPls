cmake_minimum_required(VERSION 3.11)

project(TabsPls)

set(TabsPls_Headers SmartGtk.hpp
    FileSystemDirectory.hpp

	GtkGui/FileListView.hpp
	GtkGui/DragAndDrop.hpp
    
    model/FileSystem.hpp

)

set(TabsPls_Sources FileSystemDirectory.cpp
    GtkGui/FileListView.cpp
    GtkGui/DragAndDrop.cpp

    model/FileSystem.cpp
)

add_library(TabsPls ${TabsPls_Headers} ${TabsPls_Sources})
target_include_directories(TabsPls PUBLIC .)

find_package(PkgConfig)
if(PkgConfig_FOUND)
    PKG_CHECK_MODULES(GTK3 gtk+-3.0)
    target_include_directories(TabsPls PUBLIC ${GTK3_INCLUDE_DIRS})
    target_link_libraries(TabsPls PUBLIC ${GTK3_LIBRARIES})
endif()

target_compile_features(TabsPls PUBLIC cxx_std_17)

add_subdirectory(test)

add_subdirectory(TabsPlsMain)
