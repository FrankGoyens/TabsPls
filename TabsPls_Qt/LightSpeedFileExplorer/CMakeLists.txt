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
  FutureWatchDialogWithProgressBar.hpp
  ShowIsReadySignaler.hpp
  AssociatedIconProvider.hpp
  IconRetrievalRunnable.hpp
  FlattenedDirectoryViewModel.hpp
  DirectoryChanger.hpp
  FileEntryModel.hpp
  FileBrowserViewModelSwitcher.hpp
  FileBrowserViewModelProvider.hpp
  FileRetrievalRunnable.hpp
  FileRetrievalByDispatch.hpp
  EscapePodLauncher.hpp
  TabsPlsLog.hpp
  WindowsNativeContextMenu.hpp
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
  FutureWatchDialogWithProgressBar.cpp
  ShowIsReadySignaler.cpp
  FutureWatchDialog.cpp
  FlattenedDirectoryViewModel.cpp
  FileEntryModel.cpp
  FileBrowserViewModelSwitcher.cpp
  FileRetrievalRunnable.cpp
  FileRetrievalByDispatch.cpp
  EscapePodLauncher.cpp
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
set(USE_PYTHON_COMPONENT ON CACHE BOOL "Use Python for more platform independant features, and to have user extension endpoints")
if(Python3_FOUND AND USE_PYTHON_COMPONENT)
  message(STATUS "TabsPls Python component will be used")
  target_link_libraries(LightSpeedFileExplorer PRIVATE TabsPls_Python)
else()
  message(STATUS "TabsPls Python component will NOT be used")

  #Stub the components that would otherwise be implemented by the Python components
  target_sources(LightSpeedFileExplorer PRIVATE Send2Trash_stub.cpp)
endif()

if(WIN32)
  set(USE_WINAPI_ICON_PROVIDER ON CACHE BOOL "Use the Windows API to be able to retrieve icons associated with files (for display purposes). Requires QtWinExtras.")
  if(USE_WINAPI_ICON_PROVIDER)
    message(STATUS "TabsPls Windows API icon provider will be used")
    target_sources(LightSpeedFileExplorer PRIVATE TabsPls_WinIconExtract/AssociatedIconProvider.cpp)

    target_sources(LightSpeedFileExplorer PRIVATE TabsPls_WinIconExtract/WinIconExtract/WinIconExtract.cpp)
    target_include_directories(LightSpeedFileExplorer PRIVATE TabsPls_WinIconExtract/WinIconExtract/include)
    
    #There is a non-portable include path in a Windows-specific header
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        set_source_files_properties(TabsPls_WinIconExtract/WinIconExtract/WinIconExtract.cpp PROPERTIES COMPILE_FLAGS -Wno-nonportable-include-path)
    endif()

    find_package(Qt5WinExtras CONFIG REQUIRED)
    target_link_libraries(LightSpeedFileExplorer PRIVATE Qt5::WinExtras Qt5::WinExtrasPrivate)
  endif()

  #Provision resource file for icons and such
  set(APP_ICON_RESOURCE_WINDOWS ${CMAKE_SOURCE_DIR}/../../res/boot_icon.ico)
  get_filename_component(APP_ICON_RESOURCE_WINDOWS ${APP_ICON_RESOURCE_WINDOWS} ABSOLUTE)
  configure_file(res_win32/resources.rc.template ${CMAKE_CURRENT_SOURCE_DIR}/res_win32/resources.rc)
  target_sources(LightSpeedFileExplorer PRIVATE res_win32/resources.rc)

  set(USE_WINAPI_CONTEXT_MENU ON CACHE BOOL "Use the Windows API to be able to open a context menu (like the context menu in Windows explorer).")
  if(USE_WINAPI_CONTEXT_MENU) 
    message(STATUS "TabsPls Windows API context menu will be used")
    target_sources(LightSpeedFileExplorer PRIVATE WindowsNativeContextMenu.cpp)
  endif()
endif()

if(NOT USE_WINAPI_ICON_PROVIDER)
  message(STATUS "TabsPls Windows API icon provider will NOT be used")
  target_sources(LightSpeedFileExplorer PRIVATE AssociatedIconProvider_stub.cpp)
endif()

if(NOT USE_WINAPI_CONTEXT_MENU) 
  message(STATUS "TabsPls Windows API context menu will NOT be used")
  target_sources(LightSpeedFileExplorer PRIVATE WindowsNativeContextMenu_Stub.cpp)
endif()


#Link against required Qt component
target_link_libraries(LightSpeedFileExplorer
  PRIVATE Qt5::Widgets)

#Put the icon file in the build folder
configure_file(../../res/boot_icon_32.png ${CMAKE_SOURCE_DIR}/res COPYONLY)
set(TABSPLS_APP_ICON boot_icon_32.png)
configure_file(res/resources.qrc.template ${CMAKE_SOURCE_DIR}/res/resources.qrc)
target_sources(LightSpeedFileExplorer PRIVATE ${CMAKE_SOURCE_DIR}/res/resources.qrc)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

#Build EscapePod and put it in the same directory as the LightSpeedFileExplorer executable
add_subdirectory(EscapePod)
add_custom_command(TARGET LightSpeedFileExplorer 
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:EscapePod>/$<TARGET_FILE_NAME:EscapePod> $<TARGET_FILE_DIR:LightSpeedFileExplorer>/$<TARGET_FILE_NAME:EscapePod>
    COMMENT "Copying the EscapePod executable to LightSpeedFileExplorer's output directory..."
)
add_dependencies(LightSpeedFileExplorer EscapePod)
