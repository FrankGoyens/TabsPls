project(TabsPlsTest)

find_package(GTest)

set(TabsPlsTest_Sources gtest_main.cc
    testSmartGtk.cpp
)

add_executable(TabsPlsTest ${TabsPlsTest_Sources})

target_link_libraries(TabsPlsTest TabsPls)

if(UNIX)
	target_link_libraries(TabsPlsTest pthread)
endif()
