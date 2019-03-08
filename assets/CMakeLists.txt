cmake_minimum_required(VERSION 3.12.0)
project(assets VERSION 0.1.0)

# シェーダのソースファイル名
# 末尾の.hlslを除いて記述する
set(SHADER_SOURCES
    forward_shading.vert
    forward_shading.frag
    deferred_shading_p0.vert
    deferred_shading_p0.frag
    deferred_shading_p1.vert
    deferred_shading_p1.frag
    tiled_forward_shading/p0.vert
    tiled_forward_shading/p0.frag
    tiled_forward_shading/p1.comp
    tiled_forward_shading/p2.vert
    tiled_forward_shading/p2.frag
    tiled_forward_shading/p3.vert
    tiled_forward_shading/p3.frag
    shadow_mapping_p0.vert
    shadow_mapping_p0.frag
    shadow_mapping_p1.vert
    shadow_mapping_p1.frag
)

# 使用するツールへのパスを指定する
if(NOT DXC_BINARY)
    set(DXC_BINARY $ENV{DXC_BINARY})
endif(NOT DXC_BINARY)
if (NOT SPIRV_CROSS_BINARY)
    set(SPIRV_CROSS_BINARY $ENV{SPIRV_CROSS_BINARY})
endif(NOT SPIRV_CROSS_BINARY)
# if (NOT GLSLANGVALIDATOR_BINARY)
#     set(GLSLANGVALIDATOR_BINARY $ENV{GLSLANGVALIDATOR_BINARY})
# endif(NOT GLSLANGVALIDATOR_BINARY)

# シェーダをコンパイルする
if(COMPILE_SHADERS AND DXC_BINARY AND SPIRV_CROSS_BINARY)
    set(SHADER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/assets/shaders)
    set(SHADER_BINARY_DIR ${CMAKE_BINARY_DIR}/assets/shaders)

    # 出力先のディレクトリを生成する
    file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

    foreach(SHADER_FILE ${SHADER_SOURCES})
        # シェーダプロファイルを調べる
        get_filename_component(SOURCE_EXT ${SHADER_FILE} EXT)
        if(SOURCE_EXT MATCHES "vert")
            set(SHADER_PROFILE vs_6_0)
        elseif(SOURCE_EXT MATCHES "frag")
            set(SHADER_PROFILE ps_6_0)
        elseif(SOURCE_EXT MATCHES "comp")
            set(SHADER_PROFILE cs_6_0)
        else()
            message(FATAL_ERROR "unknown shader profile: ${SOURCE_EXT}")
        endif()

        set(SHADER_SOURCE ${SHADER_SOURCE_DIR}/${SHADER_FILE}.hlsl)
        set(SHADER_BINARY ${SHADER_BINARY_DIR}/${SHADER_FILE}.spv)
        set(SHADER_BINARY_GLSL ${SHADER_BINARY_DIR}/${SHADER_FILE})
        add_custom_command(
            OUTPUT ${SHADER_BINARY}
            COMMAND ${DXC_BINARY} -Zi -Od -Zpr -I . -spirv -T ${SHADER_PROFILE} -E main -Fo ${SHADER_BINARY} ${SHADER_SOURCE}
            COMMAND ${SPIRV_CROSS_BINARY} --version 450 --combined-samplers-inherit-bindings --output ${SHADER_BINARY_GLSL} ${SHADER_BINARY}
            # COMMAND ${GLSLANGVALIDATOR_BINARY} ${SHADER_BINARY_GLSL}
            WORKING_DIRECTORY ${SHADER_SOURCE_DIR}
            DEPENDS ${SHADER_SOURCE})
        list(APPEND SHADER_BINARIES ${SHADER_BINARY})
    endforeach(SHADER_FILE)

    add_custom_target(shaders
        DEPENDS ${SHADER_BINARIES}
    )
    add_dependencies(rendering_techniques shaders)
endif()
