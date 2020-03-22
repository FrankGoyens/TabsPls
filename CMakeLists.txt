cmake_minimum_required(VERSION 3.16.5)

project(TabsPls)

find_package(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)

set(TabsPls_Headers SmartGtk.hpp
)

add_library(TabsPls STATIC ${TabsPls_Headers})

set_target_properties(TabsPls PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(TabsPls PUBLIC . ${GTK3_INCLUDE_DIRS})
target_link_libraries(TabsPls PUBLIC ${GTK3_LIBRARIES})
target_compile_features(TabsPls PUBLIC cxx_std_17)

add_subdirectory(test)

add_subdirectory(TabsPlsMain)