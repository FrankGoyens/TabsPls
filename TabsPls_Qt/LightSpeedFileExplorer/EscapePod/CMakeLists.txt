project(EscapePod)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(EscapePod main.cpp)

target_link_libraries(EscapePod PRIVATE Qt5::Widgets)
