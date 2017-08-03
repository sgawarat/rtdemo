cmake_minimum_required(VERSION 3.0.0)
project(rendering_techniques VERSION 0.0.0)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/glfw)

set(GLAD_PROFILE "core" CACHE STRING "" FORCE)
add_subdirectory(libs/glad)

set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/assimp)

include(CTest)
enable_testing()

add_executable(rendering_techniques
    src/main.cpp
    src/gui.cpp
    libs/imgui/imgui.cpp
    libs/imgui/imgui_draw.cpp
)
target_include_directories(rendering_techniques PRIVATE
    libs/imgui
    ./include
)
target_link_libraries(rendering_techniques
    glad
    glfw
    assimp
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
