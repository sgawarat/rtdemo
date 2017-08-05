cmake_minimum_required(VERSION 3.0.0)
project(rendering_techniques VERSION 0.1.0)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/glfw)

set(GLAD_PROFILE "core" CACHE STRING "" FORCE)
add_subdirectory(libs/glad)

set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/assimp)

set(SPDLOG_BUILD_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/spdlog)

include(CTest)
enable_testing()

add_executable(rendering_techniques
    src/main.cpp
    src/gui.cpp
    src/logging.cpp
    src/scene/static_scene.cpp
    src/tech/forward_shading.cpp
    libs/imgui/imgui.cpp
    libs/imgui/imgui_draw.cpp
)
target_compile_definitions(rendering_techniques PRIVATE
    GLFW_INCLUDE_NONE
)
if(WIN32)
    target_compile_definitions(rendering_techniques PRIVATE
        GLFW_EXPOSE_NATIVE_WIN32
    )
endif(WIN32)

target_include_directories(rendering_techniques PRIVATE
    libs/imgui
	libs/assimp/include
    ./include
	${CMAKE_BINARY_DIR}/libs/assimp/include
	glad
	glfw
	assimp
	spdlog
)
target_link_libraries(rendering_techniques
    glad
    glfw
    assimp
    spdlog
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
