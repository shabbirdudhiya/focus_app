<<<<<<< HEAD
cmake_minimum_required(VERSION 3.10)
project(FocusApp)

# Set Windows target
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_VERSION 10)

# Use MinGW or MSVC
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_COMPILER "g++")
    set(CMAKE_C_COMPILER "gcc")
endif()

# Add executable
add_executable(focus_app focus_app.cpp)

# Windows specific settings
target_compile_options(focus_app PRIVATE -D_UNICODE -DUNICODE)

# Link libraries
target_link_libraries(focus_app PRIVATE user32 gdi32 comctl32 shell32 psapi)

# Output directory
set_target_properties(focus_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# For MinGW, add static linking
if(MINGW)
    target_link_options(focus_app PRIVATE -static -static-libgcc -static-libstdc++)
endif()
=======
cmake_minimum_required(VERSION 3.15)

project(FocusApp LANGUAGES CXX)

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_VERSION 10.0)

# Find required packages
find_package(WindowsSDK REQUIRED)

# Set compiler flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(/O2 /W4 /WX)
    add_link_options(/SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup)
else()
    add_compile_options(/Od /W4 /WX /Zi)
    add_link_options(/SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup)
endif()

# Enable Unicode
add_compile_options(/DUNICODE /D_UNICODE /DWIN32_LEAN_AND_MEAN)

# Source files
set(SOURCES
    main.cpp
    TimerWidget.cpp
    AppMonitor.cpp
    Config.cpp
    resource.rc
)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Link libraries
link_libraries(user32 gdi32 shell32 comctl32 psapi)

# Create executable
add_executable(FocusApp ${SOURCES})

# Set output directory
set_target_properties(FocusApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    OUTPUT_NAME FocusApp
)

# Copy resources
configure_file(resource.rc ${CMAKE_BINARY_DIR}/resource.rc COPYONLY)
>>>>>>> 3a88b4d236f09c65121fcab973945bc4a006c191
