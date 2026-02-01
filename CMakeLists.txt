cmake_minimum_required(VERSION 3.26)

set(CMAKE_TOOLCHAIN_FILE "$ENV{HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(kairo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Collect all source files
file(GLOB_RECURSE ALL_SOURCES "src/*.cpp")

# Separate main sources and library sources
set(MAIN_SOURCES "")
set(LIB_SOURCES ${ALL_SOURCES})

# Add main.cpp if it exists in the root
if(EXISTS "${CMAKE_SOURCE_DIR}/main.cpp")
    list(APPEND MAIN_SOURCES "${CMAKE_SOURCE_DIR}/main.cpp")
endif()

# Create executable
find_package(unofficial-uwebsockets CONFIG REQUIRED)
add_executable(kairo ${MAIN_SOURCES} ${LIB_SOURCES})

# Tests
file(GLOB TEST_SOURCES "tests/*.cpp")

foreach(TEST_FILE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
find_package(unofficial-uwebsockets CONFIG REQUIRED)
    add_executable(${TEST_NAME} ${TEST_FILE} ${LIB_SOURCES})
    target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
endforeach()
target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::uwebsockets::uwebsockets)
