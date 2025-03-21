if(RUIN_BACKEND STREQUAL "raylib")
  set(SOURCES ruin_raylib.c)
elseif(RUIN_BACKEND STREQUAL "opengl")
  set(SOURCES ruin_opengl.c)
endif()

find_package(OpenGL REQUIRED)
if(OpenGL_FOUND)
  message("OPENGL FOUND !!! ✅")
else()
  message(FATAL_ERROR "Cannot find OPENGL ❌")
endif()

add_library(${PROJECT_NAME} STATIC ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR} PUBLIC_HEADER ruin.h)
target_include_directories(${PROJECT_NAME} PUBLIC .)

message("LINKING REQUIRED FILES 🔗")
if(RUIN_BACKEND STREQUAL "raylib")
  target_link_libraries(${PROJECT_NAME} OpenGL::GL raylib "-framework Cocoa" "-framework IOKit" "-framework CoreFoundation")
elseif(RUIN_BACKEND STREQUAL "opengl")
  target_link_libraries( ${PROJECT_NAME} OpenGL::GL glad glfw "-framework Cocoa" "-framework IOKit" "-framework CoreFoundation")
endif()

