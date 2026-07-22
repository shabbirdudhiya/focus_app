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
