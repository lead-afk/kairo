# GCC 14 toolchain for C++23 + vcpkg integration

set(CMAKE_C_COMPILER gcc-14 CACHE STRING "" FORCE)
set(CMAKE_CXX_COMPILER g++-14 CACHE STRING "" FORCE)

set(CMAKE_CXX_STANDARD 23 CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "" FORCE)
set(CMAKE_CXX_EXTENSIONS OFF CACHE BOOL "" FORCE)

if(DEFINED ENV{VCPKG_ROOT})
  set(_vcpkg_root "$ENV{VCPKG_ROOT}")
else()
  set(_vcpkg_root "$ENV{HOME}/vcpkg")
endif()

include("${_vcpkg_root}/scripts/buildsystems/vcpkg.cmake")
