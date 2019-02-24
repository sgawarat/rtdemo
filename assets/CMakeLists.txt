cmake_minimum_required(VERSION 3.12.0)
project(assets VERSION 0.1.0)

set(GLSLC_BINARY $ENV{GLSLC_BINARY})

if(COMPILE_SHADERS AND GLSLC_BINARY)
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

    # set(SHADER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/assets/shaders)
    # set(SHADER_BINARY_DIR ${CMAKE_SOURCE_DIR}/assets/shaders)
    # # set(SHADER_DISASM_DIR ${CMAKE_SOURCE_DIR}/assets/shader/disasm)

    # file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})
    # # file(MAKE_DIRECTORY ${SHADER_DISASM_DIR})

    # foreach(SHADER_FILE ${SHADER_SOURCES})
    #     set(SHADER_SOURCE ${SHADER_SOURCE_DIR}/${SHADER_FILE})
    #     set(SHADER_BINARY ${SHADER_BINARY_DIR}/${SHADER_FILE}.spv)
    #     set(SHADER_DISASM ${SHADER_DISASM_DIR}/${SHADER_FILE}.spv)
    #     add_custom_command(
    #         OUTPUT ${SHADER_BINARY}
    #         COMMAND ${GLSLC_BINARY} -G -g -e main -o ${SHADER_BINARY} ${SHADER_SOURCE}
    #         DEPENDS ${SHADER_SOURCE})
    #     list(APPEND SHADER_BINARIES ${SHADER_BINARY})
    # endforeach(SHADER_FILE)

    # add_custom_target(shaders
    #     DEPENDS ${SHADER_BINARIES}
    # )
    # add_dependencies(rendering_techniques shaders)

    add_custom_target(glsl ALL
        ${GLSLC_BINARY} ${SHADER_SOURCES}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/shaders
        COMMENT "Validating shaders")

endif()
