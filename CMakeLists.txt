cmake_minimum_required(VERSION 3.31)
project(
  ruin
  VERSION 0.0.0
  DESCRIPTION "Ruin your User INterface")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

option(ENABLE_TESTING "set on to generate binary for unit test" ON)

set(CMAKE_BUILD_TYPE "Debug")

add_compile_definitions(USE_FREETYPE)

add_subdirectory(src)
add_subdirectory(examples)
