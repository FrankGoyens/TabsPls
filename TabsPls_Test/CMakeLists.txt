cmake_minimum_required(VERSION 3.11)

project(TabsPlsTest)

enable_testing()

set(TabsPlsTest_Headers 
    FakeFileSystem.hpp
    
    TabsPlsCore_defs/FileSystemDefs.hpp
)

set(TabsPlsTest_Sources 
    testDirectoryHistoryStore.cpp
    testRobustDirectoryHistoryStore.cpp
    testFileSystemAlgorithm.cpp
    testCreateNewDirectory.cpp
    testTargetDirectoryConstraints.cpp
    testDirectoryInputAutoComplete.cpp
    testTabModel.cpp
    testSortedVector.cpp

    meta/testFakeFileSystem.cpp
    meta/testFakeFileSystemOp.cpp

    FakeFileSystem.cpp
)

add_executable(TabsPlsTest ${TabsPlsTest_Headers} ${TabsPlsTest_Sources})

target_include_directories(TabsPlsTest PRIVATE TabsPlsCore_defs)
add_subdirectory(../TabsPls_Core TabsPls_Core)
target_link_libraries(TabsPlsTest PRIVATE TabsPls_Core)
target_compile_features(TabsPlsTest PUBLIC cxx_std_17)

find_package(GTest)
add_library(GTest INTERFACE)
if(GTest_FOUND)
    if(${CMAKE_VERSION} VERSION_LESS "3.20.0")
    	target_link_libraries(GTest INTERFACE GTest::GTest GTest::Main)
    else()
	    target_link_libraries(GTest INTERFACE GTest::gtest Gtest::gtest_main)
    endif()
endif()

if(UNIX)
	target_link_libraries(TabsPlsTest PRIVATE pthread)
endif()

#Optionally include tests that use Python
set(USE_PYTHON_TESTS CACHE BOOL ON "Also include tests related to Python components")
if(USE_PYTHON_TESTS)
    set(FILESYSTEM_DEFS_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/../TabsPls_Core/TabsPls_Core_StdFileSystem/include)
    add_subdirectory(../TabsPls_Python TabsPls_Python)
    add_subdirectory(../TabsPls_Core/TabsPls_Core_StdFileSystem TabsPls_Core/TabsPls_Core_StdFileSystem)
    if(Python3_FOUND)
        message(STATUS "Python core tests will be included")
        add_executable(TabsPlsPluginsTest 
            testCatchPythonException.cpp 
            testToolbar.cpp)
        target_link_libraries(TabsPlsPluginsTest PRIVATE TabsPls_Python TabsPls_Core_StdFileSystem GTest)
        set(PLUGINS_TEST_INPUT ${CMAKE_SOURCE_DIR}/plugins_test_input)
        configure_file(TestPaths.h.in TestPaths.h @ONLY)
        target_include_directories(TabsPlsPluginsTest PRIVATE ${CMAKE_BINARY_DIR})
    else()
        message(STATUS "Python core tests will NOT be included, Python3 cannot be found")
    endif()
else()
    message(STATUS "Python core tests will NOT be included, USE_PYTHON_TESTS is disabled")
endif()

gtest_add_tests(TARGET TabsPlsTest)
gtest_add_tests(TARGET TabsPlsPluginsTest)

#Copy all Python DLLs to the build output, this is usually necessary on a Windows platform
if(WIN32 AND USE_PYTHON_TESTS AND Python3_FOUND)
    foreach(RuntimeLibraryDir ${Python3_RUNTIME_LIBRARY_DIRS})
        file(GLOB PythonDLLs "${RuntimeLibraryDir}/*.dll")
        if(PythonDLLs)
            file(COPY ${PythonDLLs} DESTINATION ${CMAKE_BINARY_DIR})
        endif()
    endforeach()
endif()
