#include <rtdemo/tech/forward_shading.hpp>
#include <fstream>
#include <vector>
#include <imgui.h>
#include <rtdemo/logging.hpp>
#include <rtdemo/managed.hpp>

namespace rtdemo {
RT_MANAGED_TECHNIQUE_INSTANCE(tech, ForwardShading);

namespace tech {
namespace {
bool read_from_file(const char* path, std::vector<char>& code) {
    std::ifstream ifs(path);
    if (!ifs) {
        RT_LOG_DEBUG("failed to open \"{}\"", path);
        return false;
    }

    ifs.seekg(0, std::ios::end);
    const auto end = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    const auto length = end - ifs.tellg();

    code.resize(length);
    ifs.read(code.data(), code.size());
    return true;
}
}  // namespace

bool ForwardShading::restore() {
    std::vector<char> code;
    code.reserve(1024);

    // load vertex shader from file
    const char* vert_path = "assets/shaders/forward_shading.vert";
    if (!read_from_file(vert_path, code)) {
        log_ = fmt::format("failed to open \"{}\"", vert_path);
        return false;
    }
    garie::VertexShader vert;
    vert.gen();
    if (!vert.compile(code.data(), code.size())) {
        GLchar info_log[1024];
        vert.get_info_log(1024, info_log);
        log_ = fmt::format("Failed to compile vertex shader: {}", info_log);
        RT_LOG_DEBUG("{}", log_.c_str());
        return false;
    }

    // load fragment shader from file
    const char* frag_path = "assets/shaders/forward_shading.frag";
    if (!read_from_file(frag_path, code)) {
        log_ = fmt::format("failed to open \"{}\"", frag_path);
        return false;
    }
    garie::FragmentShader frag;
    frag.gen();
    if (!frag.compile(code.data(), code.size())) {
        GLchar info_log[1024];
        frag.get_info_log(1024, info_log);
        log_ = fmt::format("Failed to compile fragment shader: {}", info_log);
        RT_LOG_DEBUG("{}", log_.c_str());
        return false;
    }

    // create shader program
    garie::Program prog;
    prog.gen();
    if (!prog.link(vert, frag)) {
        GLchar info_log[1024];
        prog.get_info_log(1024, info_log);
        log_ = fmt::format("Failed to link shader program: {}", info_log);
        RT_LOG_DEBUG("{}", log_.c_str());
        return false;
    }

    // finalize
    prog_ = std::move(prog);
    log_ = "succeeded";
    return true;
}

bool ForwardShading::invalidate() {
    prog_ = garie::Program();
    log_ = "not available";
    return true;
}

void ForwardShading::update() {}

void ForwardShading::update_gui() {
    ImGui::TextWrapped(log_.c_str());
}

void ForwardShading::apply(scene::Scene* scene) {
    if (prog_) prog_.use();
    if (scene) scene->draw(0);
}
}  // namespace tech
}  // namespace rtrdemo
