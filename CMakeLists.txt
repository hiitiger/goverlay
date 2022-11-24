cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(goverlay)
if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64")
    SET(OUTPUT_DIR ${ROOT_DIR}/build_x64)
else()
    SET(OUTPUT_DIR ${ROOT_DIR}/build_Win32)
endif()


set(LIBRARY_OUTPUT_PATH ${OUTPUT_DIR})
set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_DIR})

add_subdirectory(electron-overlay)
