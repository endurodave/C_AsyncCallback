# Example CMake command line to create project build files:
#
# *** Windows ***
# cmake -G "Visual Studio 17 2022" -A Win32 -B Build -S .
#
# *** Linux ***
# cmake -G "Unix Makefiles" -B Build -S .

# Specify the minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and language (C or C++)
project(C_AsyncCallback VERSION 1.0 LANGUAGES C CXX)

# Set C++ standard
#set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Collect all .cpp and *.h source files in the current directory
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/*.cpp" "${CMAKE_SOURCE_DIR}/*.h")

# Add subdirectories to include path
include_directories( 
    ${CMAKE_SOURCE_DIR}/Allocator
    ${CMAKE_SOURCE_DIR}/Callback
    ${CMAKE_SOURCE_DIR}/Examples
    ${CMAKE_SOURCE_DIR}/Port
)

# Add an executable target
add_executable(C_AsyncCallbackApp ${SOURCES})

# Add subdirectories to build
add_subdirectory(Allocator)
add_subdirectory(Callback)
add_subdirectory(Examples)
add_subdirectory(Port)

target_link_libraries(C_AsyncCallbackApp PRIVATE 
    AllocatorLib
    CallbackLib
    ExamplesLib
    PortLib
)

