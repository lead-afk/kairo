cmake_minimum_required(VERSION 3.26)
project(kairo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Collect all source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Add main.cpp if it exists in the root
if(EXISTS "${CMAKE_SOURCE_DIR}/main.cpp")
    list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/main.cpp")
endif()

# Create executable
add_executable(kairo ${SOURCES})
