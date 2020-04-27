project(TabsPlsTest)

set(TabsPlsTest_Sources gtest_main.cc
    testSmartGtk.cpp
)

add_executable(TabsPlsTest ${TabsPlsTest_Sources})

target_link_libraries(TabsPlsTest TabsPls)

find_package(GTest)
if(GTest_FOUND)
	target_link_libraries(TabsPlsTest gtest)
endif()

if(UNIX)
	target_link_libraries(TabsPlsTest pthread)
endif()
