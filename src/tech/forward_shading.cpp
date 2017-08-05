#include <rtdemo/tech/forward_shading.hpp>
#include <rtdemo/logging.hpp>

namespace rtdemo {
namespace tech {
bool ForwardShading::init() {
    const char* vert_code = R"CODE(
        #version 450
        layout(binding = 0) uniform Camera {
            mat4 wvp;
        } CAMERA;
        layout(location = 0) in vec3 position;
        void main() {
            gl_Position = CAMERA.wvp * vec4(position, 1);
        }
    )CODE";
    garie::VertexShader vert;
    vert.create();
    if (!vert.compile(vert_code)) {
        GLchar info_log[1024];
        vert.get_info_log(1024, info_log);
        RT_LOG_DEBUG("Failed to compile vertex shader: {}", info_log);
        return false;
    }

    const char* frag_code = R"CODE(
        #version 450
        layout(location = 0) out vec4 final_color;
        void main() {
            final_color = vec4(1, 0, 1, 1);
        }
    )CODE";
    garie::FragmentShader frag;
    frag.create();
    if (!frag.compile(frag_code)) {
        GLchar info_log[1024];
        frag.get_info_log(1024, info_log);
        RT_LOG_DEBUG("Failed to compile fragment shader: {}", info_log);
        return false;
    }

    garie::Program prog;
    prog.create();
    if (!prog.link(vert, frag)) {
        GLchar info_log[1024];
        prog.get_info_log(1024, info_log);
        RT_LOG_DEBUG("Failed to link shader program: {}", info_log);
        return false;
    }
    prog.uniform_block_binding(0, 0);

    prog_ = std::move(prog);
    return true;
}

void ForwardShading::apply() const noexcept {
    prog_.use();
}
}  // namespace tech
}  // namespace rtrdemo
