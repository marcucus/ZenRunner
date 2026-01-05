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
    Quick
    Widgets
)

# Include directories - Public API interfaces
include_directories(${CMAKE_SOURCE_DIR}/include)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
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

# Core Layer sources (to be implemented)
set(CORE_SOURCES
    src/core/ProcessManager.cpp
    src/core/Project.cpp
    # src/core/Workspace.cpp
    # src/core/JsonParser.cpp
    # src/core/LogBuffer.cpp
)

# Platform Layer sources (to be implemented)
set(PLATFORM_SOURCES
    src/platform/NativeEffects.cpp
    src/platform/GenericNativeEffects.cpp
    src/platform/WindowsNativeEffects.cpp
    # src/platform/PlatformEffect.cpp
    # src/platform/SystemTray.cpp
    # src/platform/NativeNotifications.cpp
    # src/platform/PlatformUtils.cpp
)

# Storage Layer sources (to be implemented)
set(STORAGE_SOURCES
    # Implementations of storage interfaces:
    # src/storage/SettingsManager.cpp
    # src/storage/ProjectRepository.cpp
    # src/storage/WorkspaceRepository.cpp
)

# UI Layer sources (to be implemented)
set(UI_SOURCES
    # QML files and view model implementations:
    # src/ui/LogViewModel.cpp
    # src/ui/ProjectViewModel.cpp
    # src/ui/WorkspaceViewModel.cpp
)

# Factory implementation (to be implemented)
set(FACTORY_SOURCES
    # src/ApplicationFactory.cpp
)

# Main executable with UI and backend
set(SOURCES
    src/main.cpp
    src/core/ProcessManager.cpp
    src/core/Project.cpp
    src/platform/NativeEffects.cpp
    src/platform/GenericNativeEffects.cpp
)

# Platform-specific sources
if(WIN32)
    list(APPEND SOURCES src/platform/WindowsNativeEffects.cpp)
elseif(APPLE)
    list(APPEND SOURCES src/platform/MacOSNativeEffects.mm)
endif()

# QML and resource files
qt_add_executable(${PROJECT_NAME}
    ${SOURCES}
)

# Add QML module for the UI
qt_add_qml_module(${PROJECT_NAME}
    URI ui
    VERSION 1.0
    QML_FILES
        src/ui/Main.qml
        src/ui/Dashboard.qml
        src/ui/LogConsole.qml
        src/ui/ProjectView.qml
        src/ui/WorkspaceManager.qml
        src/ui/components/GlassCard.qml
        src/ui/components/GlassButton.qml
    RESOURCE_PREFIX "/ui"
)

# Link Qt libraries
target_link_libraries(ZenRunner PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Widgets
)

# Target properties for performance optimization
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "ZenRunner"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Enable Link Time Optimization if supported for better performance
if(LTO_SUPPORTED)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION TRUE
    )
    message(STATUS "Link Time Optimization (LTO) enabled")
else()
    message(STATUS "Link Time Optimization (LTO) not supported: ${LTO_ERROR}")
endif()

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
    target_compile_definitions(ZenRunner PRIVATE ZENRUNNER_LINUX)
    target_compile_definitions(test_signal_latency PRIVATE ZENRUNNER_LINUX)
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
# install(TARGETS ${PROJECT_NAME}
#     RUNTIME DESTINATION bin
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
# )

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
