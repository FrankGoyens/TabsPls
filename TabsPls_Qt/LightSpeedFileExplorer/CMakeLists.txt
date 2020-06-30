cmake_minimum_required(VERSION 3.5)

project(LightSpeedFileExplorer LANGUAGES CXX)

set(TabsPlsCorePrefix ${CMAKE_SOURCE_DIR}/../../TabsPls_Core)
include(${TabsPlsCorePrefix}/provision_sources.cmake)
ProvisionCoreSources(${TabsPlsCorePrefix})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)

set(TS_FILES LightSpeedFileExplorer_en_GB.ts)

set(LightSpeedFileExplorer_Headers 
  ${TabsPls_Core_Headers}

  TabsPlsCore_defs/pch.h
  TabsPlsCore_defs/FileSystemDefs.hpp

  TabsPlsMainWindow.hpp
  FileListViewModel.hpp
  FileListTableView.hpp
  DirectoryInputField.hpp
  FileListTableViewWithFilter.hpp
)

set(LightSpeedFileExplorer_Sources
  ${TabsPls_Core_Sources}
  model/FileSystem.cpp

  main.cpp
  TabsPlsMainWindow.cpp
  FileListViewModel.cpp
  FileListTableView.cpp
  DirectoryInputField.cpp
  FileListTableViewWithFilter.cpp
)

add_executable(LightSpeedFileExplorer
  ${LightSpeedFileExplorer_Headers}
  ${LightSpeedFileExplorer_Sources}
  
  ${TS_FILES}
)

target_include_directories(LightSpeedFileExplorer 
  PRIVATE 
  ../../TabsPls_Core/include
  TabsPlsCore_defs)

target_link_libraries(LightSpeedFileExplorer
  PRIVATE Qt5::Widgets)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
