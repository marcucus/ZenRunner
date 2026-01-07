cmake_minimum_required(VERSION 3.21)

project(ZenRunner VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt Configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Check for Link Time Optimization support
include(CheckIPOSupported)
check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT LTO_ERROR)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Quick
    Widgets
    Test
)

# Include directories - Public API interfaces
include_directories(${CMAKE_SOURCE_DIR}/include)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Memory optimization flags
add_compile_options(
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-fno-exceptions>  # Disable exceptions for smaller binary
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-fdata-sections>  # Enable dead code elimination
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-ffunction-sections>
)

# Link-time optimizations for memory
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-flto>  # Link-time optimization
    )
    add_link_options(
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wl,--gc-sections>  # Remove unused sections
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-flto>
    )
endif()

# ========================================
# Layer Definitions
# ========================================
# The project is organized into distinct layers:
# - Core: Business logic, process management, data models
# - Platform: OS-specific integrations and native features
# - Storage: Persistent data storage and settings
# - UI: User interface components and view models
#
# Each layer communicates only through well-defined interfaces
# located in include/<layer>/ directories.
# ========================================

# Core Layer sources
set(CORE_SOURCES
    # Implementations of core interfaces:
    src/core/ProcessManager.cpp
    include/core/ProcessManager.h
    src/core/Project.cpp
    src/core/Workspace.cpp
    src/core/JsonParser.cpp
    src/core/LogBuffer.cpp
    src/core/MemoryMonitor.cpp
    src/core/AnsiParser.cpp
    include/core/IAnsiParser.hpp
    src/core/ProcessResourceMonitor.cpp
    include/core/ProcessResourceMonitor.h
    include/core/IProcessResourceMonitor.hpp
)

# Platform Layer sources
set(PLATFORM_SOURCES
    # Implementations of platform interfaces:
    src/platform/NativeEffects.cpp
    include/platform/NativeEffects.h
    src/platform/WindowsNativeEffects.cpp
    src/platform/GenericNativeEffects.cpp
    src/platform/SystemTray.cpp
    include/platform/SystemTray.h
    src/platform/NativePlatformManager.cpp
    include/platform/NativePlatformManager.h
    src/platform/PlatformHelper.cpp
    include/platform/PlatformHelper.h
)

# Storage Layer sources
set(STORAGE_SOURCES
    # Implementations of storage interfaces:
    src/storage/SettingsManager.cpp
    src/storage/WorkspaceRepository.cpp
    src/storage/ProjectRepository.cpp
    src/storage/ApplicationStateManager.cpp
)

# UI Layer sources
set(UI_SOURCES
    # QML files and view model implementations:
    src/ui/ILogViewModel.cpp
    include/ui/ILogViewModel.hpp
    src/ui/LogViewModel.cpp
    include/ui/IProjectManager.hpp
    src/ui/ProjectManager.cpp
    include/ui/ProjectManager.h
    src/ui/StatisticsViewModel.cpp
    include/ui/StatisticsViewModel.h
)

# Factory implementation (to be implemented)
set(FACTORY_SOURCES
    # src/ApplicationFactory.cpp
)

# Main executable
add_executable(${PROJECT_NAME}
    src/main.cpp
    ${CORE_SOURCES}
    ${PLATFORM_SOURCES}
    ${STORAGE_SOURCES}
    ${UI_SOURCES}
    ${QML_RESOURCES}
#    ${FACTORY_SOURCES}
    src/ui/resources.qrc
)

# Link Qt libraries
target_link_libraries(ZenRunner PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Widgets
)

# Target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "ZenRunner"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ========================================
# Test Executables
# ========================================

# Circular Buffer Test
add_executable(test_circular_buffer
    src/test_circular_buffer.cpp
    src/core/LogBuffer.cpp
)

target_link_libraries(test_circular_buffer PRIVATE
    Qt6::Core
)

set_target_properties(test_circular_buffer PROPERTIES
    OUTPUT_NAME "test_circular_buffer"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ANSI Parser Test
add_executable(test_ansi_parser
    src/test_ansi_parser.cpp
    src/core/AnsiParser.cpp
)

target_link_libraries(test_ansi_parser PRIVATE
    Qt6::Core
    Qt6::Gui
)

set_target_properties(test_ansi_parser PROPERTIES
    OUTPUT_NAME "test_ansi_parser"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# LogBuffer with ANSI Integration Test
add_executable(test_logbuffer_ansi_integration
    src/test_logbuffer_ansi_integration.cpp
    src/core/LogBuffer.cpp
    src/core/AnsiParser.cpp
)

target_link_libraries(test_logbuffer_ansi_integration PRIVATE
    Qt6::Core
    Qt6::Gui
)

set_target_properties(test_logbuffer_ansi_integration PROPERTIES
    OUTPUT_NAME "test_logbuffer_ansi"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Log Throttling Test
add_executable(test_log_throttling
    src/test_log_throttling.cpp
    src/core/LogBuffer.cpp
    src/core/AnsiParser.cpp
    src/ui/LogViewModel.cpp
    src/ui/ILogViewModel.cpp
    include/ui/ILogViewModel.hpp
)

target_link_libraries(test_log_throttling PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Test
)

set_target_properties(test_log_throttling PROPERTIES
    OUTPUT_NAME "test_log_throttling"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ProjectRepository Test
add_executable(test_project_repository
    src/test_project_repository.cpp
    src/storage/ProjectRepository.cpp
    src/core/Project.cpp
    src/core/JsonParser.cpp
)

target_link_libraries(test_project_repository PRIVATE
    Qt6::Core
)

set_target_properties(test_project_repository PROPERTIES
    OUTPUT_NAME "test_project_repository"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ApplicationStateManager Test
add_executable(test_application_state
    src/test_application_state.cpp
    src/storage/ApplicationStateManager.cpp
    src/storage/SettingsManager.cpp
    src/storage/WorkspaceRepository.cpp
    src/storage/ProjectRepository.cpp
    src/core/Workspace.cpp
    src/core/Project.cpp
    src/core/JsonParser.cpp
)

target_link_libraries(test_application_state PRIVATE
    Qt6::Core
)

set_target_properties(test_application_state PROPERTIES
    OUTPUT_NAME "test_application_state"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ProcessResourceMonitor Test
add_executable(test_resource_monitor
    src/test_resource_monitor.cpp
    src/core/ProcessResourceMonitor.cpp
    include/core/ProcessResourceMonitor.h
    include/core/IProcessResourceMonitor.hpp
)

target_link_libraries(test_resource_monitor PRIVATE
    Qt6::Core
)

set_target_properties(test_resource_monitor PROPERTIES
    OUTPUT_NAME "test_resource_monitor"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ========================================
# Platform-specific configurations
# ========================================
if(WIN32)
    # Windows-specific settings (Mica support, DWM API, etc.)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ZENRUNNER_WINDOWS)
    target_link_libraries(${PROJECT_NAME} PRIVATE dwmapi)
endif()

if(APPLE)
    # macOS-specific settings (Vibrancy, Notch detection, etc.)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ZENRUNNER_MACOS)
    # Objective-C++ files are automatically handled by CMake based on .mm extension
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "-framework AppKit"
        "-framework Foundation"
    )
endif()

if(UNIX AND NOT APPLE)
    # Linux-specific settings
    target_compile_definitions(${PROJECT_NAME} PRIVATE ZENRUNNER_LINUX)
endif()

# ========================================
# Testing Configuration
# ========================================
# Unit tests for each layer can be added here
# enable_testing()
# add_subdirectory(tests)

# ========================================
# Documentation
# ========================================
# Doxygen documentation can be generated from interface comments
# find_package(Doxygen)
# if(DOXYGEN_FOUND)
#     doxygen_add_docs(docs
#         ${CMAKE_SOURCE_DIR}/include
#         COMMENT "Generate API documentation"
#     )
# endif()

# ========================================
# Installation
# ========================================
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
)

# Install Qt dependencies on macOS
if(APPLE)
    install(CODE "
        include(BundleUtilities)
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app\" \"\" \"\")
    " COMPONENT Runtime)
endif()

# Install documentation
install(FILES
    ${CMAKE_SOURCE_DIR}/README.md
    ${CMAKE_SOURCE_DIR}/INSTALL.md
    DESTINATION share/doc/${PROJECT_NAME}
    COMPONENT Documentation
)

# ========================================
# Interface-only library for testing
# ========================================
# Create an interface library that other projects can link against
# This allows testing implementations without building the full application
add_library(ZenRunnerInterfaces INTERFACE)
target_include_directories(ZenRunnerInterfaces INTERFACE
    ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(ZenRunnerInterfaces INTERFACE
    Qt6::Core
    Qt6::Quick
    Qt6::Widgets
)

# ========================================
# CPack Configuration for Packaging
# ========================================
set(CPACK_PACKAGE_NAME "ZenRunner")
set(CPACK_PACKAGE_VENDOR "ZenRunner Project")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A high-performance native process manager for developers")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "ZenRunner")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Platform-specific packaging
if(WIN32)
    # NSIS installer for Windows
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "ZenRunner")
    set(CPACK_NSIS_PACKAGE_NAME "ZenRunner")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH ON)
    # Optional: Set icon if available
    if(EXISTS "${CMAKE_SOURCE_DIR}/packaging/windows/icon.ico")
        set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/packaging/windows/icon.ico")
        set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/packaging/windows/icon.ico")
    endif()
    set(CPACK_NSIS_HELP_LINK "https://github.com/marcucus/ZenRunner")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/marcucus/ZenRunner")
    set(CPACK_NSIS_CONTACT "https://github.com/marcucus/ZenRunner/issues")
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\ZenRunner.lnk' '$INSTDIR\\\\bin\\\\ZenRunner.exe'"
    )
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$SMPROGRAMS\\\\$START_MENU\\\\ZenRunner.lnk'"
    )
elseif(APPLE)
    # DMG for macOS
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "ZenRunner")
    set(CPACK_DMG_FORMAT "UDZO")
    # Optional: Set background image if available
    if(EXISTS "${CMAKE_SOURCE_DIR}/packaging/macos/background.png")
        set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_SOURCE_DIR}/packaging/macos/background.png")
    endif()
    set(CPACK_BUNDLE_NAME "ZenRunner")
    set(CPACK_BUNDLE_PLIST "${CMAKE_SOURCE_DIR}/packaging/macos/Info.plist")
elseif(UNIX)
    # DEB and RPM for Linux
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
    
    # Debian package settings
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "ZenRunner Project")
    set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt6-base, qt6-declarative")
    
    # RPM package settings
    set(CPACK_RPM_PACKAGE_GROUP "Development/Tools")
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
    set(CPACK_RPM_PACKAGE_REQUIRES "qt6-qtbase, qt6-qtdeclarative")
endif()

include(CPack)
