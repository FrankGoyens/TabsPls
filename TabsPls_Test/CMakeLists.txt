cmake_minimum_required(VERSION 3.11)

project(TabsPlsTest)

find_package(GTest)
add_library(GTest_lib INTERFACE)
add_library(GTest_main INTERFACE)
if(GTest_FOUND)
    if(${CMAKE_VERSION} VERSION_LESS 3.20.0)
        target_link_libraries(GTest_lib INTERFACE GTest::GTest)
        target_link_libraries(GTest_main INTERFACE GTest::Main)
    else()
        target_link_libraries(GTest_lib INTERFACE GTest::gtest)
        target_link_libraries(GTest_main INTERFACE GTest::gtest_main)
    endif()
endif()

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
target_link_libraries(TabsPlsTest PRIVATE TabsPls_Core GTest_lib GTest_main)
target_compile_features(TabsPlsTest PUBLIC cxx_std_17)

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
	    gtest_main_tabspls_plugin_test.cc

            testCatchPythonException.cpp 
            testToolbar.cpp)
        target_link_libraries(TabsPlsPluginsTest PRIVATE TabsPls_Python TabsPls_Core_StdFileSystem GTest_lib)
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
    message(STATUS "Attempting to copy the Python runtime DLLs from dirs: ${Python3_RUNTIME_LIBRARY_DIRS} ...")
    foreach(RuntimeLibraryDir ${Python3_RUNTIME_LIBRARY_DIRS})
        file(GLOB PythonDLLs "${RuntimeLibraryDir}/python*.dll")
        if(PythonDLLs)
	    message(STATUS "Copying ${PythonDLLs} to ${CMAKE_BINARY_DIR}")
            file(COPY ${PythonDLLs} DESTINATION ${CMAKE_BINARY_DIR})
        endif()
    endforeach()
    message(STATUS "Copying the Python 'Lib' folder to build directory to make sure the Python environment is sane when running tests...")
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${Python3_STDLIB} ${CMAKE_BINARY_DIR}/Lib 
	    OUTPUT_VARIABLE copy_python_lib_out
	    ERROR_VARIABLE copy_python_lib_out)
    message(STATUS ${copy_python_lib_out})
endif()
