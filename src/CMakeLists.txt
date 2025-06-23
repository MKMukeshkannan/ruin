add_library(${PROJECT_NAME} STATIC ./ruin.c ./base.c)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR} PUBLIC_HEADER ./ruin.h)
target_include_directories(${PROJECT_NAME} PUBLIC .)

