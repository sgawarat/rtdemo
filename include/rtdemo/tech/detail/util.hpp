#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <rtdemo/garie.hpp>
#include <rtdemo/logging.hpp>

namespace rtdemo {
namespace tech {
namespace detail {
template <GLenum TYPE> inline
garie::Shader<TYPE> compile_shader_from_file(const char* path, std::string* log_ptr = nullptr) {
    garie::Shader<TYPE> shader;
    
    std::ifstream ifs(path);
    if (!ifs) {
        if (log_ptr) *log_ptr = fmt::format("Cannot open {}", path);
        RT_LOG(error, "failed to open (path:{})", path);
        return garie::Shader<TYPE>{};
    }

    // calc file size
    ifs.seekg(0, std::ios::end);
    const auto end = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    const auto length = end - ifs.tellg();

    static std::vector<char> code;
    code.resize(length);
    ifs.read(code.data(), code.size());

    shader.gen();
    if (!shader.compile(code.data(), code.size())) {
        static GLchar info_log[1024];
        shader.get_info_log(1024, info_log);
        if (log_ptr) *log_ptr = info_log;
        RT_LOG(error, "Failed to compile shader (type:{}, info_log:{})", TYPE, info_log);
        return garie::Shader<TYPE>{};
    }

    return shader;
}

inline garie::Program link_program(const garie::VertexShader& vert, const garie::FragmentShader& frag, std::string* log_ptr = nullptr) {
    garie::Program prog;
    prog.gen();
    if (!prog.link(vert, frag)) {
        static GLchar info_log[1024];
        prog.get_info_log(1024, info_log);
        if (log_ptr) *log_ptr = info_log;
        RT_LOG(error, "Failed to link shader program (info_log:{})", info_log);
        return garie::Program();
    }
    return prog;
}

// ScreenQuad geometry for common use
const garie::VertexArray& screen_quad();

void draw_screen_quad();
}  // namespace detail
}  // namespace tech
}  // namespace rtdemo
