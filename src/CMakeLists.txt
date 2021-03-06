cmake_minimum_required(VERSION 3.15.0)
cmake_policy(SET CMP0076 NEW)

include(version.cmake)

project(f4se VERSION ${F4SE_RUNTIME_VERSION_STR})

runtime_version( F4SE_RUNTIME_VERSION_HEX )

set(CMAKE_CONFIGURATION_TYPES "Debug;Release"
    CACHE STRING "Configuration types" FORCE)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    CACHE INTERNAL "" FORCE)

# Preprocessor definitions
add_compile_definitions(
  $<$<CONFIG:Debug>:_DEBUG>
)

if (MSVC)
  add_compile_options(/FIcommon/IPrefix.h)
endif ()

include_directories(.)

add_subdirectory(common)
add_subdirectory(f4se)
