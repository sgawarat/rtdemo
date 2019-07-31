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
    src/tech/volumetric_fog.cpp
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

add_subdirectory(assets)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
