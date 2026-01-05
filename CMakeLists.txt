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
    # Implementations of core interfaces:
    # src/core/ProcessManager.cpp
    # src/core/Project.cpp
    # src/core/Workspace.cpp
    # src/core/JsonParser.cpp
    # src/core/LogBuffer.cpp
)

# Platform Layer sources (to be implemented)
set(PLATFORM_SOURCES
    # Implementations of platform interfaces:
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

# Main executable (to be configured when implementations are ready)
# add_executable(${PROJECT_NAME}
#     src/main.cpp
#     ${CORE_SOURCES}
#     ${PLATFORM_SOURCES}
#     ${STORAGE_SOURCES}
#     ${UI_SOURCES}
#     ${FACTORY_SOURCES}
# )

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Widgets
)

# Target properties
# set_target_properties(${PROJECT_NAME} PROPERTIES
#     OUTPUT_NAME "ZenRunner"
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
# )

# ========================================
# Platform-specific configurations
# ========================================
if(WIN32)
    # Windows-specific settings (Mica support, DWM API, etc.)
    # target_compile_definitions(${PROJECT_NAME} PRIVATE ZENRUNNER_WINDOWS)
    # target_link_libraries(${PROJECT_NAME} PRIVATE dwmapi)
endif()

if(APPLE)
    # macOS-specific settings (Vibrancy, Notch detection, etc.)
    # target_compile_definitions(${PROJECT_NAME} PRIVATE ZENRUNNER_MACOS)
    # Enable Objective-C++ for macOS native APIs
    # set_source_files_properties(${PLATFORM_SOURCES} PROPERTIES
    #     COMPILE_FLAGS "-x objective-c++"
    # )
    # target_link_libraries(${PROJECT_NAME} PRIVATE
    #     "-framework AppKit"
    #     "-framework Foundation"
    # )
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
