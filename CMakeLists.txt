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
)

# Storage Layer sources
set(STORAGE_SOURCES
    # Implementations of storage interfaces:
    src/storage/SettingsManager.cpp
    src/storage/WorkspaceRepository.cpp
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
