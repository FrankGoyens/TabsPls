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

  TabsPlsCore_defs/FileSystemDefs.hpp

  FileSystemDefsConversion.hpp
  TabsPlsMainWindow.hpp
  FileListViewModel.hpp
  FileListTableView.hpp
  DirectoryInputField.hpp
  FileListTableViewWithFilter.hpp
  FilterHookedFileListTableView.hpp
  FilterHookedLineEdit.hpp
  CurrentDirectoryFileOpQtImpl.hpp
  FileBrowserWidget.hpp
  FutureWatchDialog.hpp
  QObjectProgressReport.hpp
  QObjectRecycleExceptionHandler.hpp
  VoidFutureWatchDialog.hpp
  RecycleFutureWatchDialog.hpp
  FutureWatchDialogWithProgressBar.hpp
  QStringListFutureWatchDialog.hpp
  ShowIsReadySignaler.hpp
)

set(LightSpeedFileExplorer_Sources
  ${TabsPls_Core_Sources}
  model/FileSystem.cpp

  main.cpp
  FileSystemDefsConversion.cpp
  TabsPlsMainWindow.cpp
  FileListViewModel.cpp
  FileListTableView.cpp
  DirectoryInputField.cpp
  FileListTableViewWithFilter.cpp
  FilterHookedFileListTableView.cpp
  FilterHookedLineEdit.cpp
  CurrentDirectoryFileOpQtImpl.cpp
  FileSystemOp.cpp
  FileBrowserWidget.cpp
  QObjectProgressReport.cpp
  VoidFutureWatchDialog.cpp
  RecycleFutureWatchDialog.cpp
  FutureWatchDialogWithProgressBar.cpp
  QStringListFutureWatchDialog.cpp
  ShowIsReadySignaler.cpp
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

#Conditionally link agains the Python component
add_subdirectory(../../TabsPls_Python TabsPls_Python)
set(USE_PYTHON_COMPONENT "Use Python for more platform independant features, and to have user extension endpoints" CACHE BOOL ON)
if(Python3_FOUND AND USE_PYTHON_COMPONENT)
  message(STATUS "TabsPls Python component will be used")
  target_link_libraries(LightSpeedFileExplorer PRIVATE TabsPls_Python)
else()
  message(STATUS "TabsPls Python component will NOT be used")

  #Stub the components that would otherwise be implemented by the Python components
  target_sources(LightSpeedFileExplorer PRIVATE Send2Trash_stub.cpp)
endif()

#Link against required Qt component
target_link_libraries(LightSpeedFileExplorer
  PRIVATE Qt5::Widgets)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
