cmake_minimum_required(VERSION 3.11)

project(TabsPlsTest)

set(TabsPlsCorePrefix ${CMAKE_SOURCE_DIR}/../TabsPls_Core)

set(TabsPlsTest_Headers ${TabsPlsCorePrefix}/include/TabsPlsCore/FileSystemDirectory.hpp
    ${TabsPlsCorePrefix}/include/TabsPlsCore/DirectoryHistoryStore.hpp
    ${TabsPlsCorePrefix}/include/TabsPlsCore/FileSystemFilePath.hpp
    ${TabsPlsCorePrefix}/include/TabsPlsCore/FileSystem.hpp

    FakeFileSystem.hpp
)

set(TabsPlsTest_Sources gtest_main.cc
    testDirectoryHistoryStore.cpp

    meta/testFakeFileSystem.cpp

    ${TabsPlsCorePrefix}/source/FileSystemDirectory.cpp
    ${TabsPlsCorePrefix}/source/FileSystemFilePath.cpp
    ${TabsPlsCorePrefix}/source/DirectoryHistoryStore.cpp

    FakeFileSystem.cpp
)

add_executable(TabsPlsTest ${TabsPlsTest_Headers} ${TabsPlsTest_Sources})

target_include_directories(TabsPlsTest PRIVATE TabsPlsCore_defs ${TabsPlsCorePrefix}/include)
target_compile_features(TabsPlsTest PUBLIC cxx_std_17)

find_package(GTest)
if(GTest_FOUND)
	target_link_libraries(TabsPlsTest gtest)
endif()

if(UNIX)
	target_link_libraries(TabsPlsTest pthread)
endif()