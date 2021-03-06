cmake_minimum_required(VERSION 3.12.0)
project(assets VERSION 0.1.0)

# シェーダのソースファイル名
# 末尾の.hlslを除いて記述する
set(SHADER_SOURCES
    forward_shading/p0.vert
    forward_shading/p0.frag
    deferred_shading/p0.vert
    deferred_shading/p0.frag
    deferred_shading/p1.vert
    deferred_shading/p1.frag
    tiled_forward_shading/p0.vert
    tiled_forward_shading/p0.frag
    tiled_forward_shading/p1.comp
    tiled_forward_shading/p2.vert
    tiled_forward_shading/p2.frag
    tiled_forward_shading/p3.vert
    tiled_forward_shading/p3.frag
    shadow_mapping/p0.vert
    shadow_mapping/p0.frag
    shadow_mapping/p1.vert
    shadow_mapping/p1.frag
    volumetric_fog/shadow.vert
    volumetric_fog/shadow.frag
    volumetric_fog/p0.comp
    volumetric_fog/p1.comp
    volumetric_fog/p2.vert
    volumetric_fog/p2.frag
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
            set(SHADER_PROFILE vs_6_2)
        elseif(SOURCE_EXT MATCHES "frag")
            set(SHADER_PROFILE ps_6_2)
        elseif(SOURCE_EXT MATCHES "comp")
            set(SHADER_PROFILE cs_6_2)
        else()
            message(FATAL_ERROR "unknown shader profile: ${SOURCE_EXT}")
        endif()

        set(SHADER_SOURCE ${SHADER_SOURCE_DIR}/${SHADER_FILE}.hlsl)
        set(SHADER_BINARY ${SHADER_BINARY_DIR}/${SHADER_FILE}.spv)
        set(SHADER_BINARY_GLSL ${SHADER_BINARY_DIR}/${SHADER_FILE})
        get_filename_component(SHADER_SOURCE_FILE_DIR ${SHADER_SOURCE} DIRECTORY)
        set(COMMON_SHADER_SOURCES
            ${SHADER_SOURCE_DIR}/common.hlsli
            ${SHADER_SOURCE_FILE_DIR}/common.hlsli
        )
        add_custom_command(
            OUTPUT ${SHADER_BINARY}
            COMMAND ${DXC_BINARY} -Zi -Od -Zpr -I . -spirv -enable-16bit-types -T ${SHADER_PROFILE} -E main -Fo ${SHADER_BINARY} ${SHADER_SOURCE}
            COMMAND ${SPIRV_CROSS_BINARY} --version 450 --combined-samplers-inherit-bindings --output ${SHADER_BINARY_GLSL} ${SHADER_BINARY}
            # COMMAND ${GLSLANGVALIDATOR_BINARY} ${SHADER_BINARY_GLSL}
            WORKING_DIRECTORY ${SHADER_SOURCE_FILE_DIR}
            DEPENDS ${SHADER_SOURCE} ${COMMON_SHADER_SOURCES})
        list(APPEND SHADER_BINARIES ${SHADER_BINARY})
    endforeach(SHADER_FILE)

    add_custom_target(shaders
        DEPENDS ${SHADER_BINARIES}
    )
    add_dependencies(rendering_techniques shaders)
endif()
