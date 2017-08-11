#include <rtdemo/tech/forward_shading.hpp>
#include <fstream>
#include <vector>
#include <imgui.h>
#include <rtdemo/logging.hpp>
#include <rtdemo/managed.hpp>
#include <rtdemo/tech/detail/util.hpp>

namespace rtdemo {
RT_MANAGED_TECHNIQUE_INSTANCE(tech, ForwardShading);

namespace tech {
bool ForwardShading::restore() {
    garie::VertexShader vert = detail::compile_shader_from_file<GL_VERTEX_SHADER>("assets/shaders/forward_shading.vert", &log_);
    if (!vert) return false;

    garie::FragmentShader frag = detail::compile_shader_from_file<GL_FRAGMENT_SHADER>("assets/shaders/forward_shading.frag", &log_);
    if (!frag) return false;

    garie::Program prog = detail::link_program(vert, frag, &log_);
    if (!prog) return false;

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
    glEnable(GL_DEPTH_TEST);
    if (scene) scene->draw(Layout::DrawPass::GEOMETRIES);
}
}  // namespace tech
}  // namespace rtrdemo
