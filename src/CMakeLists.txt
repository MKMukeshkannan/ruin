add_library(${PROJECT_NAME} STATIC ./ruin_core.c ./ruin_widget.c ./base.c)

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR} PUBLIC_HEADER ./ruin_widget.h)
target_include_directories(${PROJECT_NAME} PUBLIC .)
target_link_libraries(${PROJECT_NAME} PUBLIC freetype)

if (ENABLE_TESTING)
    add_executable(ruin-test ruin_test.c)
    target_link_libraries(ruin-test PUBLIC criterion)
endif()
