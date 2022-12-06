#This project provisions the Python library which can be used to integrate platform-independant features or to provide users extension endpoints
project(TabsPls_Python)

set(TABSPLS_PYTHON_WILL_BE_PACKAGED OFF CACHE BOOL "Indicates that Python is to be packaged along with the application.")

if(TABSPLS_PYTHON_WILL_BE_PACKAGED)
  message(STATUS "Python is to be packaged, so a specific minor version will be searched...")
  find_package(Python3 3.10.0...<3.11.0 COMPONENTS Development)
else()
  find_package(Python3 COMPONENTS Development)
endif()

set(TabsPls_Python_Headers include/TabsPls_Python/TabsPls_Python.hpp)

set(TabsPls_Python_Sources src/TabsPls_Python.cpp 
  src/Send2Trash.cpp
  src/PythonRAII.hpp
  src/EmbeddedPython.cpp
  src/Toolbar.cpp
)

add_library(TabsPls_Python ${TabsPls_Python_Headers} ${TabsPls_Python_Sources})
target_compile_features(TabsPls_Python PRIVATE cxx_std_17)

target_include_directories(TabsPls_Python 
PUBLIC 
  include
PRIVATE 
  ../TabsPls_Core/include
)
if(FILESYSTEM_DEFS_INCLUDE_PATH)
  target_include_directories(TabsPls_Python PRIVATE ${FILESYSTEM_DEFS_INCLUDE_PATH})
else()
  message(FATAL_ERROR "Please provide an include path in the FILESYSTEM_DEFS_INCLUDE_PATH variable so that TabsPls_Python may find a FileSystemDefs.hpp header")
endif()

target_link_libraries(TabsPls_Python PUBLIC Python3::Python Python3::Module)

set(Python3_FOUND ${Python3_FOUND} PARENT_SCOPE) #expose this to the parent so that it can optionally be linked

