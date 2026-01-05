cmake_minimum_required(VERSION 3.21)

project(ZenRunner VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(CORE_SOURCES
    # Core module sources will be added here as implemented
)

set(PLATFORM_SOURCES
    src/platform/NativeEffects.cpp
    src/platform/GenericNativeEffects.cpp
)

# Add QML resources
set(QML_RESOURCES
    src/ui/resources.qrc
)

# Main executable
add_executable(${PROJECT_NAME}
    src/main.cpp
    ${CORE_SOURCES}
    ${PLATFORM_SOURCES}
    ${QML_RESOURCES}
)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Widgets
)

# Platform-specific configurations
if(WIN32)
    # Windows-specific settings (Mica support, etc.)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ZENRUNNER_WINDOWS Q_OS_WIN)
    target_sources(${PROJECT_NAME} PRIVATE
        src/platform/WindowsNativeEffects.cpp
    )
    # Link DWM API for Mica effect
    target_link_libraries(${PROJECT_NAME} PRIVATE dwmapi)
endif()

if(APPLE)
    # macOS-specific settings (Vibrancy, Notch detection, etc.)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ZENRUNNER_MACOS Q_OS_MACOS)
    target_sources(${PROJECT_NAME} PRIVATE
        src/platform/MacOSNativeEffects.mm
    )
    # Link macOS frameworks
    find_library(APPKIT_LIBRARY AppKit)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${APPKIT_LIBRARY})
    # Enable Objective-C++ for .mm files
    set_source_files_properties(
        src/platform/MacOSNativeEffects.mm
        PROPERTIES
        COMPILE_FLAGS "-x objective-c++"
    )
endif()

if(UNIX AND NOT APPLE)
    # Linux-specific settings
    target_compile_definitions(${PROJECT_NAME} PRIVATE ZENRUNNER_LINUX)
endif()

# Subdirectories for modular build (uncomment as modules are implemented)
# add_subdirectory(src/core)
# add_subdirectory(src/ui)
# add_subdirectory(src/platform)
