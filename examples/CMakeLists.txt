cmake_minimum_required(VERSION 3.31)

project(ruin-example)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SOURCES ./ruin_with_raylib.cpp)

message("COMPILING EXAMPLES 📃")

find_package(OpenGL REQUIRED)
if(OpenGL_FOUND)
  message("OPENGL FOUND !!! ✅")
else()
  message(FATAL_ERROR "Cannot find OPENGL ❌")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE ruin  OpenGL::GL raylib "-framework Cocoa" "-framework IOKit" "-framework CoreFoundation")


