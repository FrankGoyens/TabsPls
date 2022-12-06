project(TabsPls_Core_StdFileSystem)

add_library(TabsPls_Core_StdFileSystem 
    FileSystem.cpp
    FileSystemOp.cpp)

target_compile_features(TabsPls_Core_StdFileSystem PUBLIC cxx_std_17)
target_link_libraries(TabsPls_Core_StdFileSystem TabsPls_Core)
target_include_directories(TabsPls_Core_StdFileSystem PUBLIC include)