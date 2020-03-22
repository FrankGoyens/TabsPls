project(TabsPlsTest)

find_package(GTest REQUIRED)

set(TabsPlsTest_Sources gtest_main.cc
    testSmartGtk.cpp
)

add_executable(TabsPlsTest ${TabsPlsTest_Sources})

target_link_libraries(TabsPlsTest GTest TabsPls)