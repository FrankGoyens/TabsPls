project(EscapePod)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(EscapePod main.cpp)
install(TARGETS EscapePod)

target_link_libraries(EscapePod PRIVATE Qt5::Widgets)
