cmake_minimum_required(VERSION 3.11)

project(TabsPlsTest)

enable_testing()

set(TabsPlsTest_Headers 
    FakeFileSystem.hpp
    
    TabsPlsCore_defs/FileSystemDefs.hpp
)

set(TabsPlsTest_Sources 
    gtest_main.cc
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
if(GTest_FOUND)
	target_link_libraries(TabsPlsTest PRIVATE GTest::gtest)
endif()

if(UNIX)
	target_link_libraries(TabsPlsTest PRIVATE pthread)
endif()

#Optionally include tests that use Python
find_package(Python3 3.5.0...<3.11.0 COMPONENTS Development) #Using 3.11 on Windows, when running Python related tests, gives some internal Python errors (on the Github runner at least)
set(USE_PYTHON_TESTS CACHE BOOL ON "Also include tests related to Python components")
if(Python3_FOUND AND USE_PYTHON_TESTS)
    message(STATUS "Python core tests will be included")
    target_link_libraries(TabsPlsTest PRIVATE Python3::Python Python3::Module)
    target_sources(TabsPlsTest PRIVATE testCatchPythonException.cpp ${CMAKE_SOURCE_DIR}/../TabsPls_Python/src/Send2Trash.cpp)
else()
    message(STATUS "Python core tests will NOT be included")
endif()

gtest_add_tests(TARGET TabsPlsTest)

#Copy all Python DLLs to the build output, this is usually necessary on a Windows platform
if(WIN32 AND USE_PYTHON_TESTS AND Python3_FOUND)
    foreach(RuntimeLibraryDir ${Python3_RUNTIME_LIBRARY_DIRS})
        file(GLOB PythonDLLs "${RuntimeLibraryDir}/*.dll")
        if(PythonDLLs)
            file(COPY ${PythonDLLs} DESTINATION ${CMAKE_BINARY_DIR})
        endif()
    endforeach()
endif()
