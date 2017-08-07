#include <rtdemo/tech/forward_shading.hpp>
#include <fstream>
#include <vector>
#include <rtdemo/logging.hpp>

namespace rtdemo {
namespace tech {
namespace {
bool read_from_file(const char* path, std::vector<GLchar>& code) {
    std::ifstream ifs(path);
    if (!ifs) return false;

    ifs.seekg(0, std::ios::end);
    const auto end = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    const auto file_size = end - ifs.tellg();

    code.resize(file_size);
    ifs.read(code.data(), code.size());
    return true;
}
}  // namespace

bool ForwardShading::init() {
    std::vector<char> code;
    code.reserve(1024);

    const char* vert_path = "assets/shaders/forward_shading.vert";
    if (!read_from_file(vert_path, code)) {
        RT_LOG_DEBUG("failed to open: {}", vert_path);
        return false;
    }
    garie::VertexShader vert;
    vert.create();
    if (!vert.compile(code.data(), code.size())) {
        GLchar info_log[1024];
        vert.get_info_log(1024, info_log);
        info_log_ = "Failed to compile vertex shader: ";
        info_log_ += info_log;
        RT_LOG_DEBUG("Failed to compile vertex shader: {}", info_log);
        return false;
    }

    const char* frag_path = "assets/shaders/forward_shading.frag";
    if (!read_from_file(frag_path, code)) {
        RT_LOG_DEBUG("failed to open: {}", frag_path);
        return false;
    }
    garie::FragmentShader frag;
    frag.create();
    if (!frag.compile(code.data(), code.size())) {
        GLchar info_log[1024];
        frag.get_info_log(1024, info_log);
        info_log_ = "Failed to compile fragment shader: ";
        info_log_ += info_log;
        RT_LOG_DEBUG("Failed to compile fragment shader: {}", info_log);
        return false;
    }

    garie::Program prog;
    prog.create();
    if (!prog.link(vert, frag)) {
        GLchar info_log[1024];
        prog.get_info_log(1024, info_log);
        info_log_ = "Failed to link shader program: ";
        info_log_ += info_log;
        RT_LOG_DEBUG("Failed to link shader program: {}", info_log);
        return false;
    }

    prog_ = std::move(prog);
    info_log_.clear();
    return true;
}

void ForwardShading::apply() const noexcept {
    prog_.use();
}
}  // namespace tech
}  // namespace rtrdemo
