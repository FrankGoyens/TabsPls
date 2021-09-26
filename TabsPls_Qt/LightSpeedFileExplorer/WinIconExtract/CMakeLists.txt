project(WinIconExtract)

add_executable(WinIconExtract main.cpp)

target_link_libraries(WinIconExtract GdiPlus Shlwapi)
