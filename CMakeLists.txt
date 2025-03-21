cmake_minimum_required(VERSION 3.31)

project(
  ruin
  VERSION 0.0.0
  DESCRIPTION "Ruin your User INterface")

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)
if(NOT DEFINED RUIN_BACKEND)
  set(RUIN_BACKEND "raylib")
endif()
option(ENABLE_EXAMPLE "builds example binary for the given backend" ON)

add_subdirectory(src)
add_subdirectory(vendors)
if(ENABLE_EXAMPLE)
  add_subdirectory(example)
endif()
