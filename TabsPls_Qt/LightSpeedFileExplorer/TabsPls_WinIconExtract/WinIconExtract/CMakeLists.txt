project(WinIconExtract)

add_executable(WinIconExtract main.cpp include/WinIconExtract.hpp WinIconExtract.cpp)

target_include_directories(WinIconExtract PRIVATE include)
target_link_libraries(WinIconExtract GdiPlus Shlwapi)
