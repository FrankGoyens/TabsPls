project(TabsPls_GTK_Test)

set(TabsPls_GTK_Test_Sources gtest_main.cc
    testSmartGtk.cpp
)

add_executable(TabsPls_GTK_Test ${TabsPls_GTK_Test_Sources})

target_link_libraries(TabsPls_GTK_Test TabsPls)

find_package(GTest)
if(GTest_FOUND)
	target_link_libraries(TabsPls_GTK_Test gtest)
endif()

if(UNIX)
	target_link_libraries(TabsPls_GTK_Test pthread)
endif()
