cmake_minimum_required(VERSION 3.11)

project(TabsPls)

find_package(PkgConfig)
PKG_CHECK_MODULES(GTK3 gtk+-3.0)

set(TabsPls_Headers SmartGtk.hpp
	GtkGui/FileListView.hpp
	GtkGui/DragAndDrop.hpp
    
    model/FileSystem.hpp
)

set(TabsPls_Sources GtkGui/FileListView.cpp
    GtkGui/DragAndDrop.cpp

    model/FileSystem.cpp
)

add_library(TabsPls ${TabsPls_Headers} ${TabsPls_Sources})

set_target_properties(TabsPls PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(TabsPls PUBLIC . ${GTK3_INCLUDE_DIRS})
target_link_libraries(TabsPls PUBLIC ${GTK3_LIBRARIES})
target_compile_features(TabsPls PUBLIC cxx_std_17)

add_subdirectory(test)

add_subdirectory(TabsPlsMain)
