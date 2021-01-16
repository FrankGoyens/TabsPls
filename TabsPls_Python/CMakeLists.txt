#This project provisions the Python library which can be used to integrate platform-independant features or to provide users extension endpoints
project(TabsPls_Python)

find_package(Python3 COMPONENTS Development)

set(TabsPls_Python_Headers include/TabsPls_Python/TabsPls_Python.hpp 
  include/TabsPls_Python/Send2Trash.hpp
)

set(TabsPls_Python_Sources src/TabsPls_Python.cpp 
  src/Send2Trash.cpp
  src/PythonRAII.hpp
)

add_library(TabsPls_Python ${TabsPls_Python_Headers} ${TabsPls_Python_Sources}) 
target_include_directories(TabsPls_Python PUBLIC include)
target_link_libraries(TabsPls_Python PRIVATE Python3::Module)

set(Python3_FOUND ${Python3_FOUND} PARENT_SCOPE) #expose this to the parent so that it can optionally be linked

