cmake_minimum_required(VERSION 3.12.0)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake CACHE STRING "")
endif()

project(rendering_techniques VERSION 0.1.0)

find_package(spdlog CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

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
    src/tech/shadow_mapping.cpp
)
target_compile_features(rendering_techniques PRIVATE
    cxx_std_20
)
target_compile_definitions(rendering_techniques PRIVATE
    GLFW_INCLUDE_NONE
    GLM_ENABLE_EXPERIMENTAL
    SPDLOG_FMT_EXTERNAL
)
if(UNIX)
    target_compile_options(rendering_techniques PRIVATE
        -march=native
    )
elseif(WIN32)
    target_compile_options(rendering_techniques PRIVATE
        /source-charset:utf-8
    )
    target_compile_definitions(rendering_techniques PRIVATE
        WIN32_LEAN_AND_MEAN
        NOGDI
        NOMINMAX
        GLFW_EXPOSE_NATIVE_WIN32
    )
endif(UNIX)

target_include_directories(rendering_techniques PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${OPENGL_INCLUDE_DIR}
)
target_link_libraries(rendering_techniques
    imgui::imgui
    ${ASSIMP_LIBRARIES}
    glfw
    GLEW::GLEW
    ${OPENGL_gl_LIBRARY}
    spdlog::spdlog
)

if(UNIX)
    target_link_libraries(rendering_techniques
        pthread
    )
endif(UNIX)

set(SHADER_SOURCES
    forward_shading.vert
    forward_shading.frag
    deferred_shading_p0.vert
    deferred_shading_p0.frag
    deferred_shading_p1.vert
    deferred_shading_p1.frag
    tiled_forward_shading_p0.vert
    tiled_forward_shading_p0.frag
    tiled_forward_shading_p1.comp
    tiled_forward_shading_p2.vert
    tiled_forward_shading_p2.frag
    tiled_forward_shading_p3.vert
    tiled_forward_shading_p3.frag
    shadow_mapping_p0.vert
    shadow_mapping_p0.frag
    shadow_mapping_p1.vert
    shadow_mapping_p1.frag
)

set(GLSLC_BIN C:/VulkanSDK/1.1.92.1/Bin/glslangValidator.exe)
add_custom_target(glsl ALL
    ${GLSLC_BIN} ${SHADER_SOURCES}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/assets/shaders
    COMMENT "Validating shaders")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
