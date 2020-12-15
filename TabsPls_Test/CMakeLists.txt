cmake_minimum_required(VERSION 3.11)

project(TabsPlsTest)

enable_testing()

set(TabsPlsCorePrefix ${CMAKE_SOURCE_DIR}/../TabsPls_Core)
include(${TabsPlsCorePrefix}/provision_sources.cmake)
ProvisionCoreSources(${TabsPlsCorePrefix})

set(TabsPlsTest_Headers 
    ${TabsPls_Core_Headers}

    FakeFileSystem.hpp
    
    TabsPlsCore_defs/FileSystemDefs.hpp
)

set(TabsPlsTest_Sources 
    ${TabsPls_Core_Sources}

    gtest_main.cc
    testDirectoryHistoryStore.cpp
    testRobustDirectoryHistoryStore.cpp
    testFileSystemAlgorithm.cpp
    testCreateNewDirectory.cpp

    meta/testFakeFileSystem.cpp
    meta/testFakeFileSystemOp.cpp

    FakeFileSystem.cpp
)

add_executable(TabsPlsTest ${TabsPlsTest_Headers} ${TabsPlsTest_Sources})

target_include_directories(TabsPlsTest PRIVATE TabsPlsCore_defs ${TabsPlsCorePrefix}/include)
target_compile_features(TabsPlsTest PUBLIC cxx_std_17)

find_package(GTest)
if(GTest_FOUND)
	target_link_libraries(TabsPlsTest GTest::gtest)
endif()

if(UNIX)
	target_link_libraries(TabsPlsTest pthread)
endif()

gtest_add_tests(TARGET TabsPlsTest)
