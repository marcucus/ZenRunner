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

# Source files will be added as modules are implemented
set(CORE_SOURCES
    # src/core/*.cpp files will be added here
)

set(PLATFORM_SOURCES
    # src/platform/*.cpp files will be added here
)

set(UI_SOURCES
    # src/ui/*.qml files will be added here
)

# Main executable (to be configured)
# add_executable(${PROJECT_NAME}
#     src/main.cpp
#     ${CORE_SOURCES}
#     ${PLATFORM_SOURCES}
# )

# Link Qt libraries
# target_link_libraries(${PROJECT_NAME} PRIVATE
#     Qt6::Core
#     Qt6::Quick
#     Qt6::Widgets
# )

# Platform-specific configurations
if(WIN32)
    # Windows-specific settings (Mica support, etc.)
    # target_compile_definitions(${PROJECT_NAME} PRIVATE ZENRUNNER_WINDOWS)
endif()

if(APPLE)
    # macOS-specific settings (Vibrancy, Notch detection, etc.)
    # target_compile_definitions(${PROJECT_NAME} PRIVATE ZENRUNNER_MACOS)
endif()

if(UNIX AND NOT APPLE)
    # Linux-specific settings
    # target_compile_definitions(${PROJECT_NAME} PRIVATE ZENRUNNER_LINUX)
endif()

# Subdirectories for modular build (uncomment as modules are implemented)
# add_subdirectory(src/core)
# add_subdirectory(src/ui)
# add_subdirectory(src/platform)
