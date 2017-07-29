cmake_minimum_required(VERSION 3.0.0)
project(rendering_techniques VERSION 0.0.0)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/glfw)

set(GLAD_PROFILE "core" CACHE STRING "" FORCE)
add_subdirectory(libs/glad)

include(CTest)
enable_testing()

add_executable(rendering_techniques src/main.cpp)
target_link_libraries(rendering_techniques glfw)
target_link_libraries(rendering_techniques glad)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
