cmake_minimum_required(VERSION 3.0.0)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake CACHE STRING "")
endif()

project(rendering_techniques VERSION 0.1.0)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)

include(CTest)
enable_testing()

add_executable(rendering_techniques
    src/main.cpp
    src/application.cpp
    src/gui.cpp
    src/logging.cpp
    src/util.cpp
    src/scene/static_scene.cpp
    src/tech/forward_shading.cpp
    src/tech/deferred_shading.cpp
    src/tech/tiled_forward_shading.cpp
    libs/imgui/imgui.cpp
    libs/imgui/imgui_draw.cpp
)
target_compile_features(rendering_techniques PRIVATE
    cxx_std_14
)
target_compile_options(rendering_techniques PRIVATE
    -march=native
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
    ${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/libs/spdlog/include
    ${PROJECT_SOURCE_DIR}/libs/imgui
    ${PROJECT_SOURCE_DIR}/libs/glm
    ${ASSIMP_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
)
target_link_libraries(rendering_techniques
    ${ASSIMP_LIBRARIES}
    glfw
    ${GLEW_LIBRARY}
    ${OPENGL_gl_LIBRARY}
    #pthread
)

set(SHADER_SOURCES
    forward_shadering.vert
    forward_shadering.frag
    deferred_shadering_p0.vert
    deferred_shadering_p0.frag
    deferred_shadering_p1.vert
    deferred_shadering_p1.frag
    tiled_forward_shadering_p0.vert
    tiled_forward_shadering_p0.frag
    tiled_forward_shadering_p1.comp
    tiled_forward_shadering_p2.vert
    tiled_forward_shadering_p2.frag
    tiled_forward_shadering_p3.vert
    tiled_forward_shadering_p3.frag
)

# add_custom_target(glsl ALL
#     glslangValidator  ${SHADER_SOURCES}
#     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/assets/shaders
#     COMMENT "Validating shaders")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
